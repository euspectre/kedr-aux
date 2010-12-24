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

unsigned long buffer_size = BUFFER_SIZE_DEFAULT;
module_param(buffer_size, ulong, S_IRUGO);

static struct trace_buffer* trace_buffer;

//wrapper for trace_buffer_write_message()
static void rb_test_buffer_write(const char* str);

// Buffer of chars for trace output;
// used for reading from trace file.
struct read_buffer
{
    char* start;//allocated memory
    char* end;//pointer after the end of the buffer
    char* current_pos;//pointer to the first unread symbol
};

static void
read_buffer_init(struct read_buffer *read_buffer)
{
    read_buffer->start = NULL;
    read_buffer->end = NULL;
    read_buffer->current_pos = NULL;
}

static void
read_buffer_destroy(struct read_buffer *read_buffer)
{
    kfree(read_buffer->start);
}

static bool
read_buffer_is_empty(struct read_buffer* read_buffer)
{
    return read_buffer->current_pos == read_buffer->end;
}
/*
 *  Set pointer 'p' to the first unread symbol.
 * Advance current_pos pointer up to the 'count' symbols.
 * Return number of symbols, on which current_pos pointer was advanced.
 * 
 * Note: returnValue != count means that buffer was fully read.
 * 
 * !! Only one process may call this function at the moment.
 */
static size_t
read_buffer_read(struct read_buffer *read_buffer,
    size_t count, const char** p);

/*
 * If buffer is fully read, update its content in accordance with
 * the next record in the ring buffer.
 * 
 * Return number of symbols in the new buffer.
 *
 * If next record is absent and 'should_wait' is not 0,
 * wait until next record became avalaible.
 * Otherwise do nothing with buffer(and return 0).
 * 
 * On error, return negative error code.
 * 
 * !! Only one process may call this function at the moment.
 */
static ssize_t
read_buffer_update(struct read_buffer *read_buffer, int should_wait);

static struct dentry* work_dir;
static const char* work_dir_name = "rb_test";

static struct dentry* control_file;
static const char* control_file_name = "control";

static struct dentry* trace_file;
static const char* trace_file_name = "trace";

static struct dentry* reset_file;
static const char* reset_file_name = "reset";

static struct dentry* buffer_size_file;
static const char* buffer_size_file_name = "buffer_size";

static struct dentry* lost_messages_file;
static const char* lost_messages_file_name = "lost_messages";


//Add "Write" message to the trace_buffer
static ssize_t control_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos);
//Add "Read" message to the trace buffer
static ssize_t control_file_read(struct file *filp,
    char __user *buf, size_t count, loff_t * f_pos);


static struct file_operations control_file_ops =
{
    .owner = THIS_MODULE,
    .write = control_file_write,
    .read = control_file_read
};

// Trace file operations
static ssize_t trace_file_read(struct file *filp,
    char __user* buf, size_t count, loff_t *f_pos);

static int trace_file_open(struct inode *inode, struct file *filp);
static int trace_file_release(struct inode *inode, struct file *filp);

static unsigned int trace_file_poll(struct file *filp, poll_table *wait);

static struct file_operations trace_file_ops = 
{
    .owner = THIS_MODULE,
    .open = trace_file_open,
    .release = trace_file_release,
    .read = trace_file_read,
    .poll = trace_file_poll,
};

// Reset buffer file operations
static ssize_t reset_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos);

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
static ssize_t
buffer_size_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos);
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

//Need for do not worry about correct destroying things
// in case, when something wrong in 'init'.
static void destroy_all(void)
{
#define REMOVE_FILE(entry) if(entry) debugfs_remove(entry)
    REMOVE_FILE(lost_messages_file);
    REMOVE_FILE(buffer_size_file);
    REMOVE_FILE(reset_file);
    REMOVE_FILE(trace_file);
    REMOVE_FILE(control_file);
    REMOVE_FILE(work_dir);
    
#undef REMOVE_FILE
    if(trace_buffer) trace_buffer_destroy(trace_buffer);
}

static int __init
rb_test_init(void)
{
    trace_buffer = trace_buffer_alloc(buffer_size, 1);
    if(trace_buffer == NULL) return -ENOMEM;
    
    buffer_size = trace_buffer_size(trace_buffer);

    work_dir = debugfs_create_dir(work_dir_name, NULL);
    if(work_dir == NULL)
    {
        pr_err("Cannot create work directory in debugfs.");
        destroy_all();
        return -EINVAL;
    }

    control_file = debugfs_create_file(control_file_name,
        S_IRUGO | S_IWUSR | S_IWGRP,
        work_dir,
        NULL,
        &control_file_ops);
    if(control_file == NULL)
    {
        pr_err("Cannot create control file.");
        destroy_all();
        return -EINVAL;
    }

    trace_file = debugfs_create_file(trace_file_name,
        S_IRUGO,
        work_dir,
        NULL,
        &trace_file_ops);
    if(trace_file == NULL)
    {
        pr_err("Cannot create trace file.");
        destroy_all();
        return -EINVAL;
    }
    
    reset_file = debugfs_create_file(reset_file_name,
        S_IWUSR | S_IWGRP,
        work_dir,
        NULL,
        &reset_file_ops);
    if(reset_file == NULL)
    {
        pr_err("Cannot create reset file.");
        destroy_all();
        return -EINVAL;
    }

    buffer_size_file = debugfs_create_file(buffer_size_file_name,
        S_IRUGO | S_IWUSR | S_IWGRP,
        work_dir,
        NULL,
        &buffer_size_file_ops);
    if(buffer_size_file == NULL)
    {
        pr_err("Cannot create file for control size of buffer.");
        destroy_all();
        return -EINVAL;
    }
    
    lost_messages_file = debugfs_create_file(lost_messages_file_name,
        S_IRUGO,
        work_dir,
        NULL,
        &lost_messages_file_ops);
    if(lost_messages_file == NULL)
    {
        pr_err("Cannot create file for control number of messages lost.");
        destroy_all();
        return -EINVAL;
    }

    return 0;
}

void __exit
rb_test_exit(void)
{
    destroy_all();
}

module_init(rb_test_init);
module_exit(rb_test_exit);

MODULE_AUTHOR("Andrey Tsyvarev");
MODULE_DESCRIPTION("Using ring buffer for tracing example");
MODULE_LICENSE("GPL");
////////////////////////////////////


void rb_test_buffer_write(const char* str)
{
    trace_buffer_write_message(trace_buffer,
        str, strlen(str) + 1);
}

/*
 *  Set pointer 'p' to the first unread symbol.
 * Advance current_pos pointer up to the 'count' symbols.
 * Return number of symbols, on which current_pos pointer was advanced.
 * 
 * Note: returnValue != count means that buffer was fully read.
 */
static size_t
read_buffer_read(struct read_buffer *read_buffer,
    size_t count, const char** p)
{
    *p = read_buffer->current_pos;
    if((read_buffer->end - read_buffer->current_pos) < count)
        count = read_buffer->end - read_buffer->current_pos;
    read_buffer->current_pos += count;
    return count;
}



// Callback for trace_buffer_read_message.
static int read_buffer_update_process_data(const void* msg,
    size_t size, int cpu, u64 ts, void* user_data)
{
    // ts is time in nanoseconds since system starts
    u32 sec, ms;
    //snprintf-like function, which print message into string,
    //which then will be read from the trace.
#define print_msg(buffer, size) snprintf(buffer, size, "[%.03d]\t%.6lu.%.06u:\t%s\n", \
    cpu, (unsigned long)sec, (unsigned)ms, (const char*)msg)

    struct read_buffer *read_buffer = (struct read_buffer*)user_data;
    size_t read_size;
   
    sec = div_u64_rem(ts, 1000000000, &ms);
    ms /= 1000;

    read_size = print_msg(NULL, 0);//determine size of the message
    //Need to allocate buffer for message + '\0' byte, because
    //snprintf appends '\0' in any case, even if it does not need.
    read_buffer->start = krealloc(read_buffer->start, read_size,
        GFP_KERNEL);
    if(read_buffer->start)
    {
        // Real printing
        // read_size + 1 means size of message + '\0' byte
        print_msg(read_buffer->start, read_size + 1);
        // We don't want to read '\0' byte, so silently ignore it
        // (read_size without "+1")
        read_buffer->end = read_buffer->start + read_size;
        read_buffer->current_pos = read_buffer->start;
        return read_size;
    }
    else
    {
        read_buffer->end = NULL;
        read_buffer->current_pos = NULL;
        return -ENOMEM;
    }
#undef print_msg
}

/*
 * If buffer is fully read, update its content in accordance with
 * the next record in the ring buffer.
 * 
 * Return number of symbols in the new buffer.
 *
 * If next record is absent and 'should_wait' is not 0,
 * wait until next record became avalaible.
 * Otherwise do nothing with buffer(and return 0).
 * 
 * On error, return negative error code.
 */
static ssize_t
read_buffer_update(struct read_buffer *read_buffer, int should_wait)
{
    ssize_t result;
    
    if(read_buffer->current_pos != read_buffer->end)
        return read_buffer->end - read_buffer->current_pos;
    
    result = trace_buffer_read_message(trace_buffer,
        read_buffer_update_process_data,
        should_wait,
        read_buffer);
    
    if(result) return result;
    return read_buffer->end - read_buffer->current_pos;
}


// Control file operations implementation.
ssize_t control_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos)
{
    rb_test_buffer_write("Write");
    return count;
}

ssize_t control_file_read(struct file *filp,
    char __user *buf, size_t count, loff_t * f_pos)
{
    rb_test_buffer_write("Read");
    return 0;//eof
}

//Implementation of trace file operations
static int trace_file_open(struct inode *inode, struct file *filp)
{
    int result;
    
    struct read_buffer* read_buffer;

    read_buffer = kmalloc(sizeof(*read_buffer), GFP_KERNEL);
    if(read_buffer == NULL)
    {
        pr_err("trace_file_open: cannot create trace buffer.");
        return -ENOMEM;
    }
    read_buffer_init(read_buffer);
    filp->private_data = read_buffer;
    result = nonseekable_open(inode, filp);
    if(result != 0)
    {
        read_buffer_destroy(read_buffer);
        kfree(read_buffer);
    }
    return result;
}
static int trace_file_release(struct inode *inode, struct file *filp)
{
    struct read_buffer* read_buffer = filp->private_data;
    read_buffer_destroy(read_buffer);
    kfree(read_buffer);
    return 0;
}

ssize_t trace_file_read(struct file *filp,
    char __user* buf, size_t count, loff_t *f_pos)
{
    size_t size;
    struct read_buffer* read_buffer = filp->private_data;
    const char* str;

    ssize_t result;
    
    (void)f_pos;//do not use

    if(count == 0) return 0;
    result = read_buffer_read(read_buffer, count, &str);
    if(result == 0)
    {
        //update buffer and re-read it
        ssize_t error = read_buffer_update(read_buffer,
            (filp->f_flags & O_NONBLOCK) == 0);
        if(error < 0) return error;
        //now should read some chars
        result = read_buffer_read(read_buffer, count, &str);
    }
    if(result < 0) return result;// Some error occures
    size = (size_t)result;
    
    if(size == 0) return 0;//eof

    //If need, correct 'count'
    if(count > size)
        count = size;

    if(copy_to_user(buf, str, count) != 0)
        return -EFAULT;

    return count;
}

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
    int can_read = 0;
    struct read_buffer* read_buffer = filp->private_data;
    if(!read_buffer_is_empty(read_buffer))
    {
        can_read = 1;
    }
    else
    {
        struct trace_file_poll_table table;
        table.filp = filp;
        table.wait = wait;
        can_read = trace_buffer_poll_read(trace_buffer, trace_file_wait_function,
            &table);
    }
    return (can_read < 0) ? POLLERR : (can_read ? (POLLIN | POLLRDNORM) : 0);
}

// Reset buffer file operations implementation
ssize_t reset_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos)
{
    // Do nothing, resetting already performed while opening file
    return count;
}

int reset_file_open(struct inode *inode, struct file *filp)
{
    trace_buffer_reset(trace_buffer);
    return nonseekable_open(inode, filp);
}

// Buffer size file operations implementation
int
buffer_size_file_open(struct inode *inode, struct file *filp)
{
    int result;
    unsigned long size = trace_buffer_size(trace_buffer);
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
int
buffer_size_file_release(struct inode *inode, struct file *filp)
{
    char* size_str = filp->private_data;
    kfree(size_str);
    return 0;
}
ssize_t buffer_size_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos)
{
    int error = 0;
    char* str;
    unsigned long size;
    
    if(count == 0) return -EINVAL;
    str = kmalloc(count, GFP_KERNEL);
    if(str == NULL)
    {
        error = -ENOMEM;
        goto out;
    }
    if(copy_from_user(str, buf, count))
    {
        error = -EFAULT;
        goto out;
    }
    error = strict_strtoul(str, 0, &size);
    if(error) goto out;
    error = trace_buffer_resize(trace_buffer, size);
    if(error >= 0)
    {
        buffer_size = size;
        error = 0;
    }
out:
    kfree(str);
    return error ? error : count;
}
ssize_t buffer_size_file_read(struct file *filp,
    char __user* buf, size_t count, loff_t *f_pos)
{
    const char* size_str = filp->private_data;
    size_t size_len = strlen(size_str);
    
    if(count == 0) return 0;
    if(*f_pos > size_len) return -EINVAL;
    if(size_len < count + *f_pos)
        count = size_len - *f_pos;
    
    if(copy_to_user(buf, size_str + *f_pos, count))
        return -EFAULT;
    
    *f_pos += count;
    return count;
}

// Lost messages file operations implementation
int
lost_messages_file_open(struct inode *inode, struct file *filp)
{
    int result;
    unsigned long lost_messages =
        trace_buffer_lost_messages(trace_buffer);
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
    
    if(count == 0) return 0;
    if(*f_pos > len) return -EINVAL;
    if(len < count + *f_pos)
        count = len - *f_pos;
    
    if(copy_to_user(buf, str + *f_pos, count))
        return -EFAULT;
    
    *f_pos += count;
    return count;
}
