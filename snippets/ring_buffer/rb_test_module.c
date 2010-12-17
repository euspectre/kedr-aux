#include <trace_buffer.h>

#include <linux/module.h>
#include <linux/init.h>

#include <linux/mutex.h>

#include <linux/debugfs.h>

#include <linux/uaccess.h>

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

static struct file_operations trace_file_ops = 
{
    .owner = THIS_MODULE,
    .open = trace_file_open,
    .release = trace_file_release,
    .read = trace_file_read,
};

static int __init
rb_test_init(void)
{
    trace_buffer = trace_buffer_alloc(1000, 1);
    if(trace_buffer == NULL) return -ENOMEM;
    
    work_dir = debugfs_create_dir(work_dir_name, NULL);
    if(work_dir == NULL)
    {
        pr_err("Cannot create work directory in debugfs.");
        trace_buffer_destroy(trace_buffer);
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
        debugfs_remove(work_dir);
        trace_buffer_destroy(trace_buffer);
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
        debugfs_remove(control_file);
        debugfs_remove(work_dir);
        trace_buffer_destroy(trace_buffer);
        return -EINVAL;
    }

    return 0;
}

void __exit
rb_test_exit(void)
{
    debugfs_remove(trace_file);
    debugfs_remove(control_file);
    debugfs_remove(work_dir);
    trace_buffer_destroy(trace_buffer);
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
    size_t size, u64 ms, void* user_data)
{
    struct read_buffer *read_buffer = (struct read_buffer*)user_data;
    //real format of message data
    const char* str = (const char*)msg;
    //format for output message data
#define format_string "%s\n"
    size_t read_size;
   
    read_size = snprintf(NULL, 0, format_string, str) + 1;//include terminating '\0'
    read_buffer->start = krealloc(read_buffer->start, read_size,
        GFP_KERNEL);
    if(read_buffer->start)
    {
        snprintf(read_buffer->start, read_size,
            format_string, str);
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

// For prevent opening trace file,
// when it is already opened by another reader.
static atomic_t trace_file_opened = ATOMIC_INIT(0);
static int lock_buffer(void)
{
    if(atomic_cmpxchg(&trace_file_opened, 0, 1) != 0)
    {
        pr_err("Trace file cannot be opened twice at the same moment.");
        return -EINVAL;
    }
    return 0;
}
static void unlock_buffer(void)
{
    smp_wmb();
    atomic_set(&trace_file_opened, 0);
}
//Implementation of trace file operations
static int trace_file_open(struct inode *inode, struct file *filp)
{
    int result;
    
    struct read_buffer* read_buffer;

    result = lock_buffer();
    if(result) return result;

    read_buffer = kmalloc(sizeof(*read_buffer), GFP_KERNEL);
    if(read_buffer == NULL)
    {
        pr_err("trace_file_open: cannot create trace buffer.");
        unlock_buffer();
        return -ENOMEM;
    }
    read_buffer_init(read_buffer);
    filp->private_data = read_buffer;
    result = nonseekable_open(inode, filp);
    if(result != 0)
    {
        read_buffer_destroy(read_buffer);
        kfree(read_buffer);
        unlock_buffer();
    }
    return result;
}
static int trace_file_release(struct inode *inode, struct file *filp)
{
    struct read_buffer* read_buffer = filp->private_data;
    read_buffer_destroy(read_buffer);
    kfree(read_buffer);
    unlock_buffer();
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
            filp->f_flags & O_NONBLOCK);
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
