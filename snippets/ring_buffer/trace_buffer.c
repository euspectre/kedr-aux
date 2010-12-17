/*
 * Implementation of the 'trace_buffer' API.
 */

#include <linux/ring_buffer.h> /* ring buffer functions*/
#include <linux/sched.h> /* wait_event */

#include <linux/cpumask.h> /* definition of 'struct cpumask'(cpumask_t)*/
#include <linux/threads.h> /*definition of NR_CPUS macro*/


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
 * .size = ring_buffer_event_size(event),
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
            pr_err("Cannot allocate message from event.");
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
     * Number of entries, which was written to the ring buffer.
     * 
     * This number may overflow its type,
     * but it only need to be changed each new entry.
     */
    //atomic_t buffer_write_entries_number;
    /*
     * Waitqueue for buffer readers who wait for any non-empty buffer.
     */
    wait_queue_head_t trace_reader_queue;
    /*
     * Array of last messages from all possible CPUs
     */
    struct last_message last_messages[NR_CPUS];
    // caches for empty and non-empty per-cpu buffers
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
        pr_err("trace_buffer_init: Cannot allocate ring buffer.");
        kfree(trace_buffer);
        return NULL;
    }
    init_waitqueue_head(&trace_buffer->trace_reader_queue);

    //Initialize array of oldest messages
    for_each_possible_cpu(cpu)
    {
        struct last_message* last_message =
            &trace_buffer->last_messages[cpu];
        
        //now last_message_init return only 0(success)
        last_message_init(last_message,
            ring_buffer_time_stamp(trace_buffer->buffer, cpu));
    }
    
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
    //
    wake_up_interruptible(&trace_buffer->trace_reader_queue);
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
    int result;
    int cpu;
    
    int oldest_cpu = -1;//cpu which have oldest message
    int oldest_cpu_empty = -1;//cpu which doesn't have messages, and its 'ts' is the oldest one
    
    u64 oldest_ts = 0;//time stamp for oldest message
    u64 oldest_ts_empty = 0;//time stamp at which all cpu-buffers was empty
    
    struct last_message* last_messages = trace_buffer->last_messages;
    
    //Calculate statistics
    //Assume, that there are at least one cpu exist :)
    //So either subbuffers_empty or subbufers_nonempty mask is not empty.
    
    //Statistics for empty subbuffers
    if(!cpumask_empty(&trace_buffer->subbuffers_empty)) do
    {
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
                int error =
                    last_message_fill(&last_messages[cpu], ts, event);
                if(error) return error;//simply return on error
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
        // Need to wait any
        if(!should_wait) return 0;
        if(wait_event_killable(trace_buffer->trace_reader_queue,
            ring_buffer_entries(trace_buffer->buffer)  != 0))
        {
            return -ERESTARTSYS;//was interrupted
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
    
    while((oldest_cpu_empty != -1) && (oldest_ts_empty <= oldest_ts))
    {
        //Need wait for messages on some cpu's for make
        //(oldest_ts_empty > oldest_ts)
        // Do this with simple sleep
        if(!should_wait) return 0;
        if(msleep_interruptible(1))//sleep for one milisecond
        {
            return -ERESTARTSYS;//was interrupted
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
                int error =
                    last_message_fill(&last_messages[cpu], ts, event);
                if(error) return error;//simply return on error
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
    return result;
}

/*
 * Return number of messages lost due to the buffer overflow.
 */

unsigned long
trace_buffer_lost_messages(struct trace_buffer* trace_buffer)
{
    return ring_buffer_overruns(trace_buffer->buffer);
}


