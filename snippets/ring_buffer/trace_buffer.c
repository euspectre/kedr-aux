/*
 * Implementation of the 'trace_buffer' API.
 */

#include <linux/ring_buffer.h> /* ring buffer functions*/
#include <linux/delay.h> /* msleep_interruptible definition */

#include <linux/cpumask.h> /* definition of 'struct cpumask'(cpumask_t)*/
#include <linux/threads.h> /*definition of NR_CPUS macro*/

#include <linux/mutex.h> /* mutexes */

#include <linux/slab.h> /* kmalloc and others*/
/*
 * Configurable parameters for internal implementation of the buffer.
 */

/* 
 * Problem: extracting message in bloking mode, but buffer is empty.
 * 
 * Decision: extractor wait for some time, and then perform new attempt
 * to read message from buffer.
 * 
 * 'TIME_WAIT_BUFFER' is a time in ms for this waiting.
 */
#define TIME_WAIT_BUFFER 500

/*
 * Problem: extracting message in blocking mode; there are some messages
 *   in the buffer with oldest timestamp 'ts_message',
 *   but there are also empty sub-buffer with last access timestamp 
 *   'ts_access', and 'ts_access'<'ts_message'.
 * 
 *   In this case one cannot say, that message with timestamp 'ts_message'
 *   WILL the oldest message IN ANY CASE besause message with timestamp
 *   'ts_access' may appear in future in the empty subbuffer.
 *
 * Descision: extractor wait a little, and then update access timestamp
 *   in the empty subbuffer(and try to read message from it).
 *   Even the subbuffer remain to be empty, last access timestamp may
 *   exceed 'ts_message', so ordering may be performed.
 * 
 * 'TIME_WAIT_SUBBUFFER' is a time in ms of this waiting.
 * 
 * Note: this time may be 0, because for normal counting of timestamps
 *   for CPUs, if timestamp 'ts_access' evaluated after
 *   'ts_message', then automatically ts_access >= ts_message.
 * 
 */
#define TIME_WAIT_SUBBUFFER 1

/*
 * Describe last message from per-cpu buffer.
 * 
 * Because this message may be long not accessed, we cannot store
 * it as ring_buffer_event.
 * 
 * There are two different types of this struct:
 * 
 * First - for existent last message:
 * event = ring_buffer_consume(buffer, cpu, &.ts);
 * 
 * .msg = ring_buffer_event_data(event),
 * .size = ring_buffer_event_length(event),
 * .ts - time stamp of the event.
 * 
 * Second - for empty per-cpu buffer:
 * .msg = NULL,
 * .size = 0,
 * .ts - time stamp, at which buffer was definitly empty.
 */
struct last_message
{
    int cpu;
    u64 ts;
    bool is_exist;
    
    void *msg;
    size_t size;
    
    struct list_head list;//ordered array of 'struct last_message'
};

static int last_message_init(struct last_message* message, int cpu, u64 ts)
{
    message->cpu = cpu;
    message->ts = ts;
    message->is_exist = 0;
    
    message->msg = NULL;
    message->size = 0;
    return 0;
}
static void
last_message_set_timestamp(struct last_message* message, u64 ts)
{
    message->ts = ts;
}
static int
last_message_set(struct last_message* message, struct ring_buffer_event* event)
{
    size_t size;
    BUG_ON(message->is_exist);
    size = ring_buffer_event_length(event);
    message->msg = krealloc(message->msg, size, GFP_KERNEL);
    if(message->msg == NULL)
    {
        message->size = 0;
        //Cannot allocate message for return - drop it
        pr_err("last_message_fill: Cannot allocate message from event.");
        //now simply return
        return -ENOMEM;
    }
    memcpy(message->msg,
        ring_buffer_event_data(event),
        size);
    message->size = size;
    message->is_exist = 1;
    return 0;
}

static void last_message_clear(struct last_message* message)
{
    message->is_exist = 0;
    // Do not free old message contents - for realloc
    //kfree(message->msg);
    //message->msg = NULL;
    //message->size = 0;
}

static void last_message_destroy(struct last_message* message)
{
    kfree(message->msg);
}


/*
 * Struct, represented buffer which support two main operations:
 *
 * 1.Writting message into buffer
 * 2.Extract oldest message from the buffer.
 *   If buffer is empty, may wait.
 */

struct trace_buffer
{
    struct ring_buffer* buffer;

    /*
     * Array of last messages from all possible CPUs
     */
    struct last_message last_messages[NR_CPUS];
    
    struct list_head last_messages_ordered;// from the oldest timestamp to the newest one
    //number of per-cpu buffers, from which messages was readed into 'struct last_message'
    int non_empty_buffers;
    
    /*
     * Prevent concurrent reading of messages.
     */
    struct mutex read_mutex;
    /*
     * Number of messages, lost due to incorrect processing
     * of the events from ring_buffer_consume().
     * Other losts are account in ring_buffer_overruns().
     */
    unsigned long messages_lost_internal;
    // Caches for empty and non-empty per-cpu buffers
    //cpumask_t subbuffers_empty;
    //cpumask_t subbuffers_nonempty;
};

/*
 * Allocate buffer.
 * 
 * 'size' is size of the buffer created.
 * 'mode_overwrite' determine policy,
 *  when size is overflowed while writting message:
 *   if 'mode_overwrite' is 0, then newest message will be dropped.
 *   otherwise the oldest message will be dropped.
 */
struct trace_buffer* trace_buffer_alloc(
    size_t size, bool mode_overwrite)
{
    int cpu;

    struct trace_buffer* trace_buffer = kmalloc(sizeof(*trace_buffer),
        GFP_KERNEL);
    
    if(trace_buffer == NULL)
    {
        pr_err("trace_buffer_alloc: Cannot allocate trace_buffer structure.");
        return NULL;
    }
    
    trace_buffer->buffer = ring_buffer_alloc(size,
        mode_overwrite? RB_FL_OVERWRITE : 0);
    if(trace_buffer->buffer == NULL)
    {
        pr_err("trace_buffer_alloc: Cannot allocate ring buffer.");
        kfree(trace_buffer);
        return NULL;
    }

    //Initialize array of the oldest messages from per-cpu buffers
    INIT_LIST_HEAD(&trace_buffer->last_messages_ordered);
    trace_buffer->non_empty_buffers = 0;
    for_each_possible_cpu(cpu)
    {
        struct list_head* insert_point;
        struct last_message* last_message =
            &trace_buffer->last_messages[cpu];
        u64 ts = ring_buffer_time_stamp(trace_buffer->buffer, cpu);
        //now last_message_init return only 0(success)
        last_message_init(last_message, cpu, ts);
        //look for position for insert entry into the list
        list_for_each_prev(insert_point, &trace_buffer->last_messages_ordered)
        {
            if(list_entry(insert_point, struct last_message, list)->ts <= ts) break;
        }
        list_add(&last_message->list, insert_point);
    }
    
    
    mutex_init(&trace_buffer->read_mutex);
    
    trace_buffer->messages_lost_internal = 0;
    
    //cpumask_setall(&trace_buffer->subbuffers_empty);
    //cpumask_clear(&trace_buffer->subbuffers_nonempty);

    return trace_buffer;
}
/*
 * Destroy buffer, free all resources which it used.
 */
void trace_buffer_destroy(struct trace_buffer* trace_buffer)
{
    int cpu;
    
    mutex_destroy(&trace_buffer->read_mutex);
    //May be cpu-s, for which bit in subbuffers_empty is set,
    //but which messages is not freed.
    //So iterate over all possible cpu's
    for_each_possible_cpu(cpu)
    {
        struct last_message* last_message =
            &trace_buffer->last_messages[cpu];
        
        last_message_destroy(last_message);
    }
    ring_buffer_free(trace_buffer->buffer);
    kfree(trace_buffer);
}

/*
 * Write message with data 'msg' of length 'size' to the buffer.
 * May be called in the atomic context.
 */
void trace_buffer_write_message(struct trace_buffer* trace_buffer,
    const void* msg, size_t size)
{
    //need to cast msg to non-constan pointer,
    // but really its content is not changed inside function
    ring_buffer_write(trace_buffer->buffer, size, (void*)msg);
}

/*
 * Read the oldest message from the buffer, and consume it.
 * 
 * For message consumed call 'process_data':
 * 'msg' is set to the pointer to the message data.
 * 'size' is set to the size of the message,
 * 'cpu' is set to the cpu, on which message was written,
 * 'ts' is set to the timestamp of the message,
 * 'user_data' is set to the 'user_data' parameter of the function.
 * 
 * Return value, which is returned by 'process_data'.
 * 
 * If buffer is empty, and should_wait is 0,
 * return 0; otherwise wait until message will be available
 * 
 * If error occures, return negative error code.
 * 
 * Shouldn't be called in atomic context.
 */
int
trace_buffer_read_message(struct trace_buffer* trace_buffer,
    int (*process_data)(const void* msg, size_t size, int cpu, u64 ts, void* user_data),
    int should_wait,
    void* user_data)
{
    int result = 0;
    
    /*
     * Flags for cpu's, on which cpu-buffer was already updated.
     *
     * If we need to update cpu-buffer for cpu, for which its flag was set, we need wait.
     *
     * These flags should be cleared after every mutex_unlock.
     */
    cpumask_t subbuffers_updated;
    
    struct last_message* oldest_message;
    
    if(mutex_lock_killable(&trace_buffer->read_mutex))
        return -ERESTARTSYS;
    cpumask_clear(&subbuffers_updated);
    // Try to determine oldest message in the buffer(from all cpu's)
    for(oldest_message = list_first_entry(&trace_buffer->last_messages_ordered, struct last_message, list);
        !oldest_message->is_exist;
        oldest_message = list_first_entry(&trace_buffer->last_messages_ordered, struct last_message, list))
    {
        // Cannot determine latest message - need to update timestamp
        int cpu = oldest_message->cpu;
        u64 ts;
        struct ring_buffer_event* event;
        struct list_head* insert_point;
        
        if(cpumask_test_cpu(cpu, &subbuffers_updated))
        {
            //field 'non_empty_buffers' should be tested under the lock
            bool non_empty_buffers = trace_buffer->non_empty_buffers;
            //need to wait
            if(!should_wait)
            {
                result = -EAGAIN;
                goto out;
            }
            cpumask_clear(&subbuffers_updated);
            mutex_unlock(&trace_buffer->read_mutex);
            
            if(non_empty_buffers == 0)
            {
                if(msleep_interruptible(TIME_WAIT_BUFFER))
                {
                    //mutex already dropped, so simply return
                    return -ERESTARTSYS;//was interrupted
                }
            }
            else
            {
                if(msleep_interruptible(TIME_WAIT_SUBBUFFER))
                {
                    //mutex already dropped, so simply return
                    return -ERESTARTSYS;//was interrupted
                }
            }
            if(mutex_lock_killable(&trace_buffer->read_mutex))
                return -ERESTARTSYS;
            //mutex was dropped, so need to extract the oldest message again
            continue;
        }

        ts = ring_buffer_time_stamp(trace_buffer->buffer, cpu);
        event = ring_buffer_consume(trace_buffer->buffer, cpu, &ts);
        last_message_set_timestamp(oldest_message, ts);
        //rearrange 'oldest_message'
        list_del(&oldest_message->list);
        list_for_each_prev(insert_point, &trace_buffer->last_messages_ordered)
        {
            if(list_entry(insert_point, struct last_message, list)->ts <= ts) break;
        }
        list_add(&oldest_message->list, insert_point);
        //mark cpu as 'updated'
        cpumask_set_cpu(cpu, &subbuffers_updated);
        
        if(event)
        {
            result = last_message_set(oldest_message, event);
            if(result)
            {
                trace_buffer->messages_lost_internal++;
                goto out;
            }
            trace_buffer->non_empty_buffers++;
        }
    }
    // Determine oldest message
    result = process_data(oldest_message->msg,
        oldest_message->size,
        oldest_message->cpu,
        oldest_message->ts,
        user_data);
    //Remove oldest message
    last_message_clear(oldest_message);
    trace_buffer->non_empty_buffers--;
out:
    mutex_unlock(&trace_buffer->read_mutex);
    return result;
}

/*
 * Return number of messages lost due to the buffer overflow.
 */

unsigned long
trace_buffer_lost_messages(struct trace_buffer* trace_buffer)
{
    return ring_buffer_overruns(trace_buffer->buffer)
            + trace_buffer->messages_lost_internal;
}

/*
 * Reset trace in the buffer.
 * 
 * Return 0 on success, negative error code otherwise.
 */
int
trace_buffer_reset(struct trace_buffer* trace_buffer)
{
    int cpu;
    if(mutex_lock_interruptible(&trace_buffer->read_mutex))
    {
        return -ERESTARTSYS;
    }
    ring_buffer_reset(trace_buffer->buffer);
    //Clear last messages
    INIT_LIST_HEAD(&trace_buffer->last_messages_ordered);
    trace_buffer->non_empty_buffers = 0;

    for_each_possible_cpu(cpu)
    {
        struct list_head* insert_point;
        struct last_message* last_message =
            &trace_buffer->last_messages[cpu];
        u64 ts = ring_buffer_time_stamp(trace_buffer->buffer, cpu);
        last_message_clear(last_message);
        last_message_set_timestamp(last_message, ts);
        //look for position for insert entry into the list
        list_for_each_prev(insert_point, &trace_buffer->last_messages_ordered)
        {
            if(list_entry(insert_point, struct last_message, list)->ts <= ts) break;
        }
        list_add(&last_message->list, insert_point);
    }

    trace_buffer->messages_lost_internal = 0;
    mutex_unlock(&trace_buffer->read_mutex);
    return 0;
}

/*
 * Return size of buffer in bytes.
 */
unsigned long
trace_buffer_size(struct trace_buffer* trace_buffer)
{
    return ring_buffer_size(trace_buffer->buffer);
}

/*
 * Change size of the buffer.
 *
 * Current messages in the buffer may be silently lost.
 * (in current implementation buffer is forcibly reseted).
 *
 * Return new size on success, negative error code otherwise.
 */

int trace_buffer_resize(struct trace_buffer* trace_buffer,
    unsigned long size)
{
    int result;
    int cpu;
    if(mutex_lock_interruptible(&trace_buffer->read_mutex))
    {
        return -ERESTARTSYS;
    }
    result = ring_buffer_resize(trace_buffer->buffer, size);
    ring_buffer_reset(trace_buffer->buffer);
    //Clear last messages
    INIT_LIST_HEAD(&trace_buffer->last_messages_ordered);
    trace_buffer->non_empty_buffers = 0;

    for_each_possible_cpu(cpu)
    {
        struct list_head* insert_point;
        struct last_message* last_message =
            &trace_buffer->last_messages[cpu];
        u64 ts = ring_buffer_time_stamp(trace_buffer->buffer, cpu);
        last_message_clear(last_message);
        last_message_set_timestamp(last_message, ts);
        //look for position for insert entry into the list
        list_for_each_prev(insert_point, &trace_buffer->last_messages_ordered)
        {
            if(list_entry(insert_point, struct last_message, list)->ts <= ts) break;
        }
        list_add(&last_message->list, insert_point);
    }

    trace_buffer->messages_lost_internal = 0;

    mutex_unlock(&trace_buffer->read_mutex);
    return result;
}
