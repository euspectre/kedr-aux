/*
 * Implementation of the 'trace_buffer' API.
 */

#include <linux/ring_buffer.h> /* ring buffer functions*/
#include <linux/delay.h> /* msleep_interruptible definition */

#include <linux/cpumask.h> /* definition of 'struct cpumask'(cpumask_t)*/
#include <linux/threads.h> /*definition of NR_CPUS macro*/

#include <linux/mutex.h> /* mutexes */
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
 * First - for existant last message:
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
    u64 ts;
    void *msg;
    size_t size;
    /*
     *  Waitqueue for buffer reader,
     *  which wait particular sub-buffer for being non-empty.
     */
    //wait_queue_head_t trace_reader_queue;
};

static int last_message_init(struct last_message* message, u64 ts)
{
    message->ts = ts;
    message->msg = NULL;
    message->size = 0;
    return 0;
}
static int last_message_fill(struct last_message* message,
    u64 ts, struct ring_buffer_event* event)
{
    message->ts = ts;
    if(event)
    {
        size_t size = ring_buffer_event_length(event);
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
    }
    else
    {
        kfree(message->msg);
        message->msg = NULL;
        message->size = 0;
    }
    return 0;
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
    cpumask_t subbuffers_empty;
    cpumask_t subbuffers_nonempty;
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

    //Initialize array of oldest messages
    for_each_possible_cpu(cpu)
    {
        struct last_message* last_message =
            &trace_buffer->last_messages[cpu];
        
        //now last_message_init return only 0(success)
        last_message_init(last_message,
            ring_buffer_time_stamp(trace_buffer->buffer, cpu));
    }
    mutex_init(&trace_buffer->read_mutex);
    
    trace_buffer->messages_lost_internal = 0;
    
    cpumask_setall(&trace_buffer->subbuffers_empty);
    cpumask_clear(&trace_buffer->subbuffers_nonempty);

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
    int (*process_data)(const void* msg, size_t size, u64 ts, void* user_data),
    int should_wait,
    void* user_data)
{
    int result = 0;
    int cpu;
    
    int oldest_cpu = -1;//cpu which have oldest message
    int oldest_cpu_empty = -1;//cpu which doesn't have messages, and its 'ts' is the oldest one
    
    u64 oldest_ts = 0;//time stamp for oldest message
    u64 oldest_ts_empty = 0;//time stamp at which all cpu-buffers was empty
    
    struct last_message* last_messages = trace_buffer->last_messages;
    
    if(mutex_lock_killable(&trace_buffer->read_mutex))
        return -ERESTARTSYS;
    
    //Calculate statistics
    //Assume, that there are at least one cpu exist :)
    //So either subbuffers_empty or subbufers_nonempty mask is not empty.
    
    //Statistics for empty subbuffers
    if(!cpumask_empty(&trace_buffer->subbuffers_empty)) do
    {
        oldest_cpu_empty = -1;//reset statistic at the start of the cycle
        for_each_cpu(cpu, &trace_buffer->subbuffers_empty)
        {
            //update time stamp and try to read message from the buffer
            struct ring_buffer_event* event;
            u64 ts =
                ring_buffer_time_stamp(trace_buffer->buffer, cpu);
            event = ring_buffer_consume(trace_buffer->buffer,
                cpu, &ts);
            last_messages[cpu].ts = ts;
            if(event)
            {
                result = 
                    last_message_fill(&last_messages[cpu], ts, event);
                if(result)
                {
                    trace_buffer->messages_lost_internal++;
                    goto out;
                }
                cpumask_clear_cpu(cpu, &trace_buffer->subbuffers_empty);
                cpumask_set_cpu(cpu, &trace_buffer->subbuffers_nonempty);
            }
            else if((oldest_cpu_empty == -1) || (oldest_ts_empty > ts))
            {
                oldest_cpu_empty = cpu;
                oldest_ts_empty = ts;
            }
        }
        if(!cpumask_empty(&trace_buffer->subbuffers_nonempty))
            break;//next step
        //There is no available messages at all
        // Need to wait any - use simple sleep
        if(!should_wait) goto out;
        if(msleep_interruptible(TIME_WAIT_BUFFER))
        {
            result = -ERESTARTSYS;//was interrupted
            goto out;
        }
    }while(1);
    // Now there is at least one cpu, for which messsage exists.
    // Calculate statistics for those processes.
    for_each_cpu(cpu, &trace_buffer->subbuffers_nonempty)
    {
        u64 ts = last_messages[cpu].ts;
        if((oldest_cpu == -1) || (oldest_ts > ts))
        {
            oldest_cpu = cpu;
            oldest_ts = ts;
        }
    }
    
    while((oldest_cpu_empty != -1) && (oldest_ts_empty < oldest_ts))
    {
        //Need wait for messages on some cpu's for make
        //(oldest_ts_empty > oldest_ts)
        // Do this with simple sleep
        if(!should_wait) goto out;
        if(msleep_interruptible(TIME_WAIT_SUBBUFFER))
        {
            result = -ERESTARTSYS;//was interrupted
            goto out;
        }

        // Clear statistic for empty buffers,
        // verify messages in them and update all statistic
        oldest_cpu_empty = -1;
       
        for_each_cpu(cpu, &trace_buffer->subbuffers_empty)
        {
            //update time stamp and try to read message from the buffer
            struct ring_buffer_event* event;
            u64 ts =
                ring_buffer_time_stamp(trace_buffer->buffer, cpu);
            event = ring_buffer_consume(trace_buffer->buffer,
                cpu, &ts);
            last_messages[cpu].ts = ts;
            if(event)
            {
                result =
                    last_message_fill(&last_messages[cpu], ts, event);
                if(result)
                {
                    trace_buffer->messages_lost_internal++;
                    goto out;
                }
                cpumask_clear_cpu(cpu, &trace_buffer->subbuffers_empty);
                cpumask_set_cpu(cpu, &trace_buffer->subbuffers_nonempty);
                //update statistic for non-empty buffers
                if(oldest_ts > ts)
                {
                    oldest_cpu = cpu;
                    oldest_ts = ts;
                }
            }
            else if((oldest_cpu_empty == -1) || (oldest_ts_empty > ts))
            {
                //update statistic for empty buffers
                oldest_cpu_empty = cpu;
                oldest_ts_empty = ts;
            }
        }
    }
    //now either there is no empty buffers at all, or time stamp
    //of this buffers exceed timestamp of non-empty buffers.
    result = process_data(last_messages[oldest_cpu].msg,
        last_messages[oldest_cpu].size,
        last_messages[oldest_cpu].ts,
        user_data);
    cpumask_clear_cpu(oldest_cpu, &trace_buffer->subbuffers_nonempty);
    cpumask_set_cpu(oldest_cpu, &trace_buffer->subbuffers_empty);

    //Corresponded cpu-buffer already marked as empty.
    //So do not remove message from it(for make krealloc using its memory)
    //last_message_fill(&last_messages[oldest_cpu], oldest_ts, NULL);

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
    if(mutex_lock_interruptible(&trace_buffer->read_mutex))
    {
        return -ERESTARTSYS;
    }
    ring_buffer_reset(trace_buffer->buffer);
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
 * Change size of the buffer, without resetting it.
 *
 * (But messages may be lost due to buffer overflow,
 * or due to writting during resizing).
 * 
 * Return new size on success, negative error code otherwise.
 */

int trace_buffer_resize(struct trace_buffer* trace_buffer,
    unsigned long size)
{
    int result;
    if(mutex_lock_interruptible(&trace_buffer->read_mutex))
    {
        return -ERESTARTSYS;
    }
    result = ring_buffer_resize(trace_buffer->buffer, size);
    mutex_unlock(&trace_buffer->read_mutex);
    return result;
}
