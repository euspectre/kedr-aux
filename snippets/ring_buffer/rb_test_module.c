#include <trace_buffer.h>

#include <linux/module.h>
#include <linux/init.h>

#include <linux/mutex.h>

#include <linux/debugfs.h>

#include <linux/uaccess.h>

#include <linux/moduleparam.h>

#include <linux/slab.h> /*kmalloc and others*/

#include <linux/poll.h>

#define BUFFER_SIZE_DEFAULT 1000

//Initial buffer size
unsigned long buffer_size = BUFFER_SIZE_DEFAULT;
module_param(buffer_size, ulong, S_IRUGO);

// Names of files
static const char* work_dir_name = "rb_test";
static const char* control_file_name = "control";
static const char* trace_file_name = "trace";
static const char* reset_file_name = "reset";
static const char* buffer_size_file_name = "buffer_size";
static const char* lost_messages_file_name = "lost_messages";
//
static struct dentry* control_file;
static struct dentry* work_dir;

/*
 * Struct, which implements trace in simple form.
 */
struct trace
{
    //buffer with 'archived' messages
    struct trace_buffer* trace_buffer;
    //last message in 'plain' form
    char* start;//allocated memory
    char* end;//pointer after the end of the buffer
    char* current_pos;//pointer to the first unread symbol
    //protect the message in 'plain' form from concurrent access.
    struct mutex m;
    //control files
    struct dentry* trace_file;
    struct dentry* reset_file;
    struct dentry* buffer_size_file;
    struct dentry* lost_messages_file;
};

/*
 * Simple writer to the trace.
 * 
 * Write null-terminated string into trace buffer.
 * 
 * In real application it may write any sequence of bytes.
 */
static void rb_test_buffer_write(struct trace* trace, const char* str);

/*
 * Interpretator of the trace content.
 * 
 * In the current implementation it interpret message as string
 * (see comments to the rb_test_buffer_write()) and write as text
 * with cpu number and timestamp.
 * 
 * Real application may interpret the message as some struct,
 * and choose format according to some field(s) of this struct.
 * 
 * Return 1 if plain message become non-empty, negative error code otherwise.
 * 
 * NOTE: Do not return 0 for do not confuse with buffer emptiness
 * in non-block reading(see trace_buffer_read_message()).
 */
static int trace_process_data(const void* msg,
    size_t size, int cpu, u64 ts, bool* consume, void* user_data);


// Global trace object.
static struct trace trace;

/*
 * Initialize trace for collecting messages.
 */
static int trace_init(struct trace* trace, size_t buffer_size,
    bool mode_overwrite, struct dentry* work_dir);

static void trace_destroy(struct trace* trace);

static int control_file_open(struct inode *inode, struct file *filp);

//Add "Write" message to the trace_buffer
static ssize_t control_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos);
//Add "Read" message to the trace buffer
static ssize_t control_file_read(struct file *filp,
    char __user *buf, size_t count, loff_t * f_pos);


static struct file_operations control_file_ops =
{
    .owner = THIS_MODULE,
    .open = control_file_open,
    .write = control_file_write,
    .read = control_file_read
};


// Trace file operations
static int trace_file_open(struct inode *inode, struct file *filp);
// Consume messages from the trace.
static ssize_t trace_file_read(struct file *filp,
    char __user* buf, size_t count, loff_t *f_pos);
// Wait until message will be available in the trace.
static unsigned int trace_file_poll(struct file *filp, poll_table *wait);

static struct file_operations trace_file_ops = 
{
    .owner = THIS_MODULE,
    .open = trace_file_open,
    .read = trace_file_read,
    .poll = trace_file_poll,
};

// Reset buffer file operations
// Do nothing
static ssize_t reset_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos);
// Reset trace.
static int reset_file_open(struct inode *inode, struct file *filp);

static struct file_operations reset_file_ops = 
{
    .owner = THIS_MODULE,
    .open = reset_file_open,
    .write = reset_file_write,
};

// Buffer size file operations
static int
buffer_size_file_open(struct inode *inode, struct file *filp);
static int
buffer_size_file_release(struct inode *inode, struct file *filp);
// Set size of the trace buffer
static ssize_t
buffer_size_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos);
// Get size of the trace buffer
static ssize_t
buffer_size_file_read(struct file *filp,
    char __user* buf, size_t count, loff_t *f_pos);

static struct file_operations buffer_size_file_ops = 
{
    .owner = THIS_MODULE,
    .open = buffer_size_file_open,
    .release = buffer_size_file_release,
    .read = buffer_size_file_read,
    .write = buffer_size_file_write,
};

// Lost messages file operations
static int
lost_messages_file_open(struct inode *inode, struct file *filp);
static int
lost_messages_file_release(struct inode *inode, struct file *filp);
// Return number of messages which lost.
static ssize_t
lost_messages_file_read(struct file *filp,
    char __user* buf, size_t count, loff_t *f_pos);

static struct file_operations lost_messages_file_ops = 
{
    .owner = THIS_MODULE,
    .open = lost_messages_file_open,
    .release = lost_messages_file_release,
    .read = lost_messages_file_read,
};

static int __init
rb_test_init(void)
{
    int result;
    work_dir = debugfs_create_dir(work_dir_name, NULL);
    if(work_dir == NULL)
    {
        pr_err("Cannot create work directory in debugfs.");
        return -EINVAL;
    }

    result = trace_init(&trace, buffer_size, 1, work_dir);
    if(result)
    {
        debugfs_remove(work_dir);
        return result;
    }

    control_file = debugfs_create_file(control_file_name,
        S_IRUGO | S_IWUSR | S_IWGRP,
        work_dir,
        &trace,
        &control_file_ops);
    if(control_file == NULL)
    {
        pr_err("Cannot create control file.");
        trace_destroy(&trace);
        debugfs_remove(work_dir);
        return -EINVAL;
    }

    return 0;
}

void __exit
rb_test_exit(void)
{
    debugfs_remove(control_file);
    trace_destroy(&trace);
    debugfs_remove(work_dir);
}

module_init(rb_test_init);
module_exit(rb_test_exit);

MODULE_AUTHOR("Andrey Tsyvarev");
MODULE_DESCRIPTION("Using ring buffer for tracing example");
MODULE_LICENSE("GPL");
////////////////////////////////////

// Control file operations implementation.
int control_file_open(struct inode *inode, struct file *filp)
{
    filp->private_data = inode->i_private;
    return nonseekable_open(inode,filp);
}
ssize_t control_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos)
{
    struct trace* trace = filp->private_data;
    rb_test_buffer_write(trace, count <= 10 ? "Write" : "Write large");
    return count;
}

ssize_t control_file_read(struct file *filp,
    char __user *buf, size_t count, loff_t * f_pos)
{
    struct trace* trace = filp->private_data;
    rb_test_buffer_write(trace, "Read");
    return 0;//eof
}

//Implementation of trace file operations
static int trace_file_open(struct inode *inode, struct file *filp)
{
    filp->private_data = inode->i_private;
    return nonseekable_open(inode, filp);
}


ssize_t trace_file_read(struct file *filp,
    char __user* buf, size_t count, loff_t *f_pos)
{
    struct trace* trace = (struct trace*)filp->private_data;
    
    if(mutex_lock_interruptible(&trace->m))
        return -ERESTARTSYS;
    
    while(trace->end == trace->current_pos)
    {
        ssize_t error;
        mutex_unlock(&trace->m);
        error = trace_buffer_read_message(trace->trace_buffer,
            trace_process_data,
            !(filp->f_flags & O_NONBLOCK),
            trace);
        if(error < 0)
            return error;
        if(error == 0)
            return -EAGAIN;
        if(mutex_lock_interruptible(&trace->m))
            return -ERESTARTSYS;
        //need to verify that plain message is not empty again,
        //because someone may read it while we reaquire lock
    }
    if(count > (trace->end - trace->current_pos))
        count = trace->end - trace->current_pos;
    
    if(copy_to_user(buf, trace->current_pos, count) != 0)
    {
        mutex_unlock(&trace->m);
        return -EFAULT;
    }
    trace->current_pos += count;
    mutex_unlock(&trace->m);
    return count;
}

/*
 * Auxiliary struct for implement file's polling method via trace_buffer_poll_read.
 */
struct trace_file_poll_table
{
    struct file *filp;
    poll_table *wait;
};

static void trace_file_wait_function(wait_queue_head_t *q, void* data)
{
    struct trace_file_poll_table* table = (struct trace_file_poll_table*)data;
    poll_wait(table->filp, q, table->wait);
}

static unsigned int trace_file_poll(struct file *filp, poll_table *wait)
{
    int can_read = 0;//1 - can read, 0 - cannot read, <0 - error
    struct trace* trace = filp->private_data;
    
    // Fast path, without lock(!)
    can_read = (trace->current_pos != trace->end) ? 1 : 0;

    if(!can_read)
    {
        struct trace_file_poll_table table;
        table.filp = filp;
        table.wait = wait;
        can_read = trace_buffer_poll_read(trace->trace_buffer, trace_file_wait_function,
            &table);
    }
    
    return (can_read < 0) ? POLLERR : (can_read ? (POLLIN | POLLRDNORM) : 0);
}

// Reset buffer file operations implementation
ssize_t reset_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos)
{
    // Do nothing, trace reseting already performed while opening file
    return count;
}

int reset_file_open(struct inode *inode, struct file *filp)
{
    struct trace* trace = inode->i_private;
    if(mutex_lock_interruptible(&trace->m))
        return -ERESTARTSYS;
    kfree(trace->start);
    trace->start = NULL;
    trace->end = NULL;
    trace->current_pos = NULL;
    
    mutex_unlock(&trace->m);

    trace_buffer_reset(trace->trace_buffer);

    return nonseekable_open(inode, filp);
}

// Buffer size file operations implementation
int
buffer_size_file_open(struct inode *inode, struct file *filp)
{
    struct trace* trace = inode->i_private;
    
    //Form private data depended from access mode.
    switch(filp->f_flags & O_ACCMODE)
    {
    case O_RDONLY:
    {
        int result;
        unsigned long size = trace_buffer_size(trace->trace_buffer);
        size_t size_len;
        char* size_str;
        size_len = snprintf(NULL, 0, "%lu\n", size);
        size_str = kmalloc(size_len + 1, GFP_KERNEL);
        if(size_str == NULL)
        {
            pr_err("buffer_size_file_open: Cannot allocate string.");
            return -ENOMEM;
        }
        snprintf(size_str, size_len + 1, "%lu\n", size);
        filp->private_data = size_str;
        result = nonseekable_open(inode, filp);
        if(result)
        {
            kfree(size_str);
        }
        return result;
    }
    break;
    case O_WRONLY:
        filp->private_data = trace;
        return nonseekable_open(inode, filp);
    break;
    case O_RDWR:
        pr_err("Simultaneous reading from and writing into size file has no sence.");
        return -EINVAL;
    break;
    default:
        pr_err("size file should be opened either for reading or for writing.");
        return -EINVAL;
    }
}
int
buffer_size_file_release(struct inode *inode, struct file *filp)
{
    if((filp->f_flags & O_ACCMODE) == O_RDONLY)
    {
        char* size_str = filp->private_data;
        kfree(size_str);
    }
    return 0;
}
ssize_t buffer_size_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos)
{
    int error = 0;
    unsigned long size;
    
    struct trace* trace = filp->private_data;
    
    if(count == 0) return -EINVAL;
    {
        char* str = kmalloc(count + 1, GFP_KERNEL);
        if(str == NULL)
        {
            return -ENOMEM;
        }
        if(copy_from_user(str, buf, count))
        {
            kfree(str);
            return -EFAULT;
        }
        str[count] = '\0';
        error = strict_strtoul(str, 0, &size);
        kfree(str);
        if(error) return error;
    }
    if(mutex_lock_interruptible(&trace->m))
    {
        mutex_unlock(&trace->m);
        return -ERESTARTSYS;
    }
    kfree(trace->start);
    trace->start = NULL;
    trace->end = NULL;
    trace->current_pos = NULL;
    
    mutex_unlock(&trace->m);

    error = trace_buffer_resize(trace->trace_buffer, size);
    if(error >= 0)
    {
        error = 0;
    }
    return error ? error : count;
}
ssize_t buffer_size_file_read(struct file *filp,
    char __user* buf, size_t count, loff_t *f_pos)
{
    const char* size_str = filp->private_data;
    size_t size_len = strlen(size_str);
    
    return simple_read_from_buffer(buf, count, f_pos, size_str, size_len);
}

// Lost messages file operations implementation
int
lost_messages_file_open(struct inode *inode, struct file *filp)
{
    int result;
    struct trace* trace = (struct trace*)inode->i_private;
    unsigned long lost_messages =
        trace_buffer_lost_messages(trace->trace_buffer);
    size_t len;
    char* str;
    len = snprintf(NULL, 0, "%lu\n", lost_messages);
    str = kmalloc(len + 1, GFP_KERNEL);
    if(str == NULL)
    {
        pr_err("lost_messages_file_open: Cannot allocate string.");
        return -ENOMEM;
    }
    snprintf(str, len + 1, "%lu\n", lost_messages);
    filp->private_data = str;
    result = nonseekable_open(inode, filp);
    if(result)
    {
        kfree(str);
    }
    return result;
}
int
lost_messages_file_release(struct inode *inode, struct file *filp)
{
    char* str = filp->private_data;
    kfree(str);
    return 0;
}
ssize_t
lost_messages_file_read(struct file *filp,
    char __user* buf, size_t count, loff_t *f_pos)
{
    const char* str = filp->private_data;
    size_t len = strlen(str);
    
    return simple_read_from_buffer(buf, count, f_pos, str, len);
}

int trace_init(struct trace* trace, size_t buffer_size,
    bool mode_overwrite, struct dentry* work_dir)
{
    memset(trace, 0, sizeof(*trace));
    mutex_init(&trace->m);
    
    trace->trace_buffer = trace_buffer_alloc(buffer_size, 1);
    if(trace->trace_buffer == NULL) return -ENOMEM;
    // Create files
    trace->trace_file = debugfs_create_file(trace_file_name,
        S_IRUGO,
        work_dir,
        trace,
        &trace_file_ops);
    if(trace->trace_file == NULL)
    {
        pr_err("Cannot create trace file.");
        trace_destroy(trace);
        return -EINVAL;
    }
    
    trace->reset_file = debugfs_create_file(reset_file_name,
        S_IWUSR | S_IWGRP,
        work_dir,
        trace,
        &reset_file_ops);
    if(trace->reset_file == NULL)
    {
        pr_err("Cannot create reset file.");
        trace_destroy(trace);
        return -EINVAL;
    }

    trace->buffer_size_file = debugfs_create_file(buffer_size_file_name,
        S_IRUGO | S_IWUSR | S_IWGRP,
        work_dir,
        trace,
        &buffer_size_file_ops);
    if(trace->buffer_size_file == NULL)
    {
        pr_err("Cannot create file for control size of buffer.");
        trace_destroy(trace);
        return -EINVAL;
    }
    
    trace->lost_messages_file = debugfs_create_file(lost_messages_file_name,
        S_IRUGO,
        work_dir,
        trace,
        &lost_messages_file_ops);
    if(trace->lost_messages_file == NULL)
    {
        pr_err("Cannot create file for control number of messages lost.");
        trace_destroy(trace);
        return -EINVAL;
    }

    return 0;
}

void trace_destroy(struct trace* trace)
{
#define REMOVE_FILE(entry) if(entry) debugfs_remove(entry)
    REMOVE_FILE(trace->lost_messages_file);
    REMOVE_FILE(trace->buffer_size_file);
    REMOVE_FILE(trace->reset_file);
    REMOVE_FILE(trace->trace_file);
#undef REMOVE_FILE
    if(trace->trace_buffer)
        trace_buffer_destroy(trace->trace_buffer);
    
    kfree(trace->start);
    
    mutex_destroy(&trace->m);
}

void rb_test_buffer_write(struct trace* trace, const char* str)
{
    trace_buffer_write_message(trace->trace_buffer,
        str, strlen(str) + 1);
}

static int trace_process_data(const void* msg,
    size_t size, int cpu, u64 ts, bool *consume, void* user_data)
{
    // ts is time in nanoseconds since system starts
    u32 sec, ms;
    //snprintf-like function, which print message into string,
    //which then will be read from the trace.
#define print_msg(buffer, size) snprintf(buffer, size, "[%.03d]\t%.6lu.%.06u:\t%s\n", \
    cpu, (unsigned long)sec, (unsigned)ms, (const char*)msg)

    struct trace *trace = (struct trace*)user_data;
    size_t read_size;
   
    sec = div_u64_rem(ts, 1000000000, &ms);
    ms /= 1000;

    if(mutex_lock_interruptible(&trace->m))
    {
        return -ERESTARTSYS;
    }
    if(trace->current_pos != trace->end)
    {
        /*
         * Someone already update plain message, while we reaquiring lock.
         * So, silently ignore updating.
         */
        mutex_unlock(&trace->m);
        return 1;
    }
    
    read_size = print_msg(NULL, 0);//determine size of the message
    //Need to allocate buffer for message + '\0' byte, because
    //snprintf appends '\0' in any case, even if it does not need.
    trace->start = krealloc(trace->start, read_size + 1,
        GFP_KERNEL);
    if(trace->start)
    {
        // Real printing
        // read_size + 1 means size of message + '\0' byte
        print_msg(trace->start, read_size + 1);
        // We don't want to read '\0' byte, so silently ignore it
        // (read_size without "+1")
        trace->end = trace->start + read_size;
        trace->current_pos = trace->start;
        mutex_unlock(&trace->m);
        *consume = 1;//message is processed
        return 1;
    }
    mutex_unlock(&trace->m);
    return -ENOMEM;
#undef print_msg
}