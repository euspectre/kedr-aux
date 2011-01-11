#include <trace_file.h>

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
//static const char* trace_file_name = "trace";
static const char* reset_file_name = "reset";
static const char* buffer_size_file_name = "buffer_size";
static const char* lost_messages_file_name = "lost_messages";

//
static struct dentry* work_dir;
static struct dentry* control_file;
static struct dentry* reset_file;
static struct dentry* buffer_size_file;
static struct dentry* lost_messages_file;

// Global trace_file object.
static struct trace_file* trace_file;

/*
 * Simple writer to the trace.
 * 
 * Write null-terminated string into trace buffer.
 * 
 * In real application it may write any sequence of bytes.
 */
static void rb_test_trace_write(struct trace_file* trace_file, const char* str);

/*
 * Interpretator of the trace content.
 * 
 * In the current implementation it interpret message as string
 * (see comments to the rb_test_trace_write()) and write as text
 * with cpu number and timestamp.
 * 
 * Real application may interpret the message as some struct,
 * and choose format according to some field(s) of this struct.
 */

int trace_print_message(char* str, size_t size,
    const void* msg, size_t msg_size, int cpu, u64 ts, void* user_data);

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
    work_dir = debugfs_create_dir(work_dir_name, NULL);
    if(work_dir == NULL)
    {
        pr_err("Cannot create work directory in debugfs.");
        return -EINVAL;
    }

    trace_file = trace_file_create(buffer_size, 1,
        work_dir, THIS_MODULE,
        trace_print_message, NULL);
    
    if(trace_file == NULL)
    {
        debugfs_remove(work_dir);
        return -EINVAL;
    }

    control_file = debugfs_create_file(control_file_name,
        S_IRUGO | S_IWUSR | S_IWGRP,
        work_dir,
        trace_file,
        &control_file_ops);
    if(control_file == NULL)
    {
        pr_err("Cannot create control file.");
        trace_file_destroy(trace_file);
        debugfs_remove(work_dir);
        return -EINVAL;
    }

    reset_file = debugfs_create_file(reset_file_name,
        S_IWUSR | S_IWGRP,
        work_dir,
        trace_file,
        &reset_file_ops);
    if(reset_file == NULL)
    {
        pr_err("Cannot create reset file.");
        
        debugfs_remove(control_file);
        trace_file_destroy(trace_file);
        debugfs_remove(work_dir);

        return -EINVAL;
    }

    buffer_size_file = debugfs_create_file(buffer_size_file_name,
        S_IRUGO | S_IWUSR | S_IWGRP,
        work_dir,
        trace_file,
        &buffer_size_file_ops);
    if(buffer_size_file == NULL)
    {
        pr_err("Cannot create file for control size of buffer.");

        debugfs_remove(reset_file);
        debugfs_remove(control_file);
        trace_file_destroy(trace_file);
        debugfs_remove(work_dir);

        return -EINVAL;
    }
    
    lost_messages_file = debugfs_create_file(lost_messages_file_name,
        S_IRUGO,
        work_dir,
        trace_file,
        &lost_messages_file_ops);
    if(lost_messages_file == NULL)
    {
        pr_err("Cannot create file for control number of messages lost.");

        debugfs_remove(buffer_size_file);
        debugfs_remove(reset_file);
        debugfs_remove(control_file);
        trace_file_destroy(trace_file);
        debugfs_remove(work_dir);

        return -EINVAL;
    }

    return 0;
}

void __exit
rb_test_exit(void)
{
    debugfs_remove(lost_messages_file);
    debugfs_remove(buffer_size_file);
    debugfs_remove(reset_file);
    debugfs_remove(control_file);
    trace_file_destroy(trace_file);
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
    return nonseekable_open(inode,filp);
}
ssize_t control_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos)
{
    struct trace_file* trace_file =
        (struct trace_file*)filp->f_dentry->d_inode->i_private;
    rb_test_trace_write(trace_file, count <= 10 ? "Write" : "Write large");
    return count;
}

ssize_t control_file_read(struct file *filp,
    char __user *buf, size_t count, loff_t * f_pos)
{
    struct trace_file* trace_file =
        (struct trace_file*)filp->f_dentry->d_inode->i_private;
    rb_test_trace_write(trace_file, "Read");
    return 0;//eof
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
    if((filp->f_flags & O_ACCMODE) != O_RDONLY)
    {
        struct trace_file* trace_file =
            (struct trace_file*)inode->i_private;
        trace_file_reset(trace_file);
    }
    return nonseekable_open(inode, filp);
}

// Buffer size file operations implementation
int
buffer_size_file_open(struct inode *inode, struct file *filp)
{
    int result;
    if((filp->f_flags & O_ACCMODE) != O_WRONLY)
    {
        struct trace_file* trace_file =
            (struct trace_file*)inode->i_private;
        
        unsigned long size = trace_file_size(trace_file);
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
    }
    result = nonseekable_open(inode, filp);
    if(result)
    {
        kfree(filp->private_data);
    }
    return result;
}
int
buffer_size_file_release(struct inode *inode, struct file *filp)
{
    kfree(filp->private_data);
    return 0;
}
ssize_t buffer_size_file_write(struct file *filp,
    const char __user *buf, size_t count, loff_t * f_pos)
{
    int error = 0;
    unsigned long size;
    
    struct trace_file* trace_file =
        (struct trace_file*)filp->f_dentry->d_inode->i_private;

    
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
    error = trace_file_size_set(trace_file, size);
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
    size_t len;
    char* str;

    struct trace_file* trace_file =
        (struct trace_file*)inode->i_private;

    unsigned long lost_messages =
        trace_file_lost_messages(trace_file);

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

///////////////////////
void rb_test_trace_write(struct trace_file* trace_file, const char* str)
{
    trace_file_write_message(trace_file,
        str, strlen(str) + 1);
}

int trace_print_message(char* str, size_t size,
    const void* msg, size_t msg_size, int cpu, u64 ts, void* user_data)
{
    // ts is time in nanoseconds since system starts
    u32 sec, ms;
   
    sec = div_u64_rem(ts, 1000000000, &ms);
    ms /= 1000;

    (void)user_data;

    return snprintf(str, size, "[%.03d]\t%.6lu.%.06u:\t%s\n",
        cpu, (unsigned long)sec, (unsigned)ms, (const char*)msg);
}