#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */

#include <linux/module.h>
    
#include <linux/uaccess.h> /* copy_*_user functions */

#include <linux/mutex.h>

#include <linux/debugfs.h> /* control file will be create on debugfs*/

#include <linux/string.h> /* strlen, strcpy */

MODULE_AUTHOR("Tsyvarev");
MODULE_LICENSE("GPL");

/*
 * Characteristic, exposed into user space.
 *
 * In our case - string which contain name of current object.
 *
 * This should be non-empty string.
 * (Correct processing wrintting of empty string into file is non-trivial task,
 * because it may be just opening file with O_TRUNC flag).
 */
static char* object_name = NULL;
// Protect 'object_name' from concurrent reads and writes.
DEFINE_MUTEX(object_name_mutex);

/*
 * Value of characteristic(object name) which is used as default - 
 * e.g. value of characteristic before the first setting,
 * 
 * It may meen absence of something. 
 */
static const char* object_name_default = "noname";

/*
 * Directory which will contatins our files(in our case - 1 file).
 */
struct dentry* module_dir;

/*
 * Control file.
 * 
 * In our case, reflect 'object_name' variable.
 */
struct dentry* object_name_file;

///////////////////Auxiliary functions////////////////////////////////

/*
 * Return current characteristic of module.
 *
 * In our case - name of object.
 *
 * Should be executed under mutes taken.
 */

static const char* get_object_name_internal(void);

/*
 * Perform 'reaction' of module on user-space impact.
 *
 * In our case - set object name.
 *
 * Should be executed under mutes taken.
 *
 * Return 0 on success, negative error code otherwise.
 */
static int set_object_name_internal(const char* new_name);



///////////////////////////File operations///////////////////////

// Helper functions, which used in implementation of file operations

//Helper for read operation of file. 'As if' it reads file, contatining string.
static ssize_t string_operation_read(const char* str, char __user *buf, size_t count, 
	loff_t *f_pos);

//Helper for llseek operation of file. Help to user space utilities to find out size of file.
static loff_t string_operation_llseek (const char* str, loff_t *f_pos, loff_t offset, int whence);

// Helper function for write operation of file. Allocate buffer, which contain writting string.
// On success(non-negative value is returned), out_str should be freed when no longer needed.
static ssize_t
string_operation_write(char** out_str, const char __user *buf,
    size_t count, loff_t *f_pos);



/////////////Implementation of file operations/////////////////////////
/*
 * For prevent unspecified behaviour of default implementation of llseek,
 * define our one.
 *
 * It may be used by user-space utils for determine size of file.
 */
static loff_t
object_name_file_llseek(struct file *filp, loff_t off, int whence)
{
    loff_t result;

    if(mutex_lock_killable(&object_name_mutex))
    {
        pr_debug("llseek() operation was killed.");
        return -EAGAIN;
    }
    result = string_operation_llseek(get_object_name_internal(), &filp->f_pos, off, whence);
        
    mutex_unlock(&object_name_mutex);
    
    return result;
}

/*
 * Get current characteristic of the module.
 *
 * In our case - return name of object.
 */
static ssize_t 
object_name_file_read(struct file *filp, char __user *buf, size_t count, 
	loff_t *f_pos)
{
    ssize_t result;
    
    if(mutex_lock_killable(&object_name_mutex))
    {
        pr_debug("read() operation was killed.");
        return -EAGAIN;
    }
    result = string_operation_read(get_object_name_internal(), buf, count, f_pos);
        
    mutex_unlock(&object_name_mutex);
    
    return result;
}

/*
 * Force module from user space to do something.
 *
 * In our case - set name of object to string, which is written.
 */
static ssize_t 
object_name_file_write(struct file *filp, const char __user *buf,
    size_t count, loff_t *f_pos)
{
    char* write_str;

    ssize_t result = string_operation_write(&write_str, buf, count, f_pos);
    
    if(result < 0) return result;//error

    if(mutex_lock_killable(&object_name_mutex))
    {
        pr_debug("write() operation was killed.");
        kfree(write_str);
        return -EAGAIN;
    }
    result = set_object_name_internal(write_str);
    mutex_unlock(&object_name_mutex);

    kfree(write_str);
    return result ? result : count;
}

static struct file_operations object_name_file_operations =
{
    .owner = THIS_MODULE,//prevent module from unload while file is opened.
    .llseek = object_name_file_llseek, //size
    .read = object_name_file_read, //get current name
    .write = object_name_file_write, //set current name
};


/////////////////////////////////////////////////////////////////////////////

static int __init
this_module_init(void)
{
    object_name = kmalloc(strlen(object_name_default) + 1, GFP_KERNEL);
    if(object_name == NULL)
    {
        pr_err("Cannot allocate name.");
        return -1;
    }
    strcpy(object_name, object_name_default);

    module_dir = debugfs_create_dir("static_control_files_example", NULL);
    if(module_dir == NULL)
    {
        pr_err("Cannot create root directory in debugfs for service.");
        kfree(object_name);
        return -1;
    }
    object_name_file = debugfs_create_file("current_name",
        S_IRUGO | S_IWUSR | S_IWGRP,
        module_dir,
        NULL,
        &object_name_file_operations
        );
    if(object_name_file == NULL)
    {
        pr_err("Cannot create control file.");
        debugfs_remove(module_dir);
        kfree(object_name);
        return -1;
    }
    
    return 0;
}

static void
this_module_exit(void)
{
    debugfs_remove(object_name_file);
    debugfs_remove(module_dir);
    kfree(object_name);
}
module_init(this_module_init);
module_exit(this_module_exit);

////////////////////Implementation of auxiliary functions////////////////////////////////

/*
 * Return current characteristic of module.
 *
 * In our case - name of object.
 */

const char* get_object_name_internal(void)
{
    return object_name;
}

/*
 * Perform 'reaction' of module on user-space impact.
 *
 * In our case - set object_name.
 * Should be executed under mutes taken.
 *
 * Return 0 on success, negative error code otherwise.
 */
int set_object_name_internal(const char* new_name)
{
    char* new_name_instance;
    
    if(*new_name == '\0') return -EINVAL;//name should be non-empty
    
    new_name_instance = kmalloc(strlen(new_name) + 1, GFP_KERNEL);
    if(new_name_instance == NULL) return -ENOMEM;
    strcpy(new_name_instance, new_name);
        
    kfree(object_name);
    object_name = new_name_instance;

    pr_debug("New name of object is '%s'.", object_name);
    return 0;
}

//Helper for read operation of file. 'As if' it reads file, contatining string.
ssize_t string_operation_read(const char* str, char __user *buf, size_t count, 
	loff_t *f_pos)
{
    //length of 'file'(include terminating '\0')
    size_t size = strlen(str) + 1;
    //whether position out of range
    if((*f_pos < 0) || (*f_pos > size)) return -EINVAL;
    if(*f_pos == size) return 0;// eof

    if(count + *f_pos > size)
        count = size - *f_pos;
    if(copy_to_user(buf, str + *f_pos, count) != 0)
        return -EFAULT;
    
    *f_pos += count;
    return count;
}

//Helper for llseek operation of file. Help to user space utilities to find out size of file.
loff_t string_operation_llseek (const char* str, loff_t *f_pos, loff_t offset, int whence)
{
    loff_t new_offset;
    size_t size = strlen(str) + 1;
    switch(whence)
    {
    case 0: /* SEEK_SET */
        new_offset = offset;
    break;
    case 1: /* SEEK_CUR */
        new_offset = *f_pos + offset;
    break;
    case 2: /* SEEK_END */
        new_offset = size + offset;
    break;
    default: /* can't happen */
        return -EINVAL;
    };
    if(new_offset < 0) return -EINVAL;
    if(new_offset > size) new_offset = size;//eof
    
    *f_pos = new_offset;
    //returning value is offset from the beginning, filp->f_pos, generally speaking, may be any.
    return new_offset;
}

ssize_t
string_operation_write(char** out_str, const char __user *buf,
    size_t count, loff_t *f_pos)
{
    char* buffer;

    if(count == 0)
    {
        pr_err("write: 'count' shouldn't be 0.");
        return -EINVAL;
    }

    /*
     * Feature of control files.
     *
     * Because writting to such files is really command to the module to do something,
     * and successive reading from this file return total effect of this command.
     * it is meaningless to process writting not from the start.
     *
     * In other words, writting always affect to the global content of the file.
     */
    if(*f_pos != 0)
    {
        pr_err("Partial rewritting is not allowed.");
        return -EINVAL;
    }
    //Allocate buffer for writting value - for its preprocessing.
    buffer = kmalloc(count + 1, GFP_KERNEL);
    if(buffer == NULL)
    {
        pr_err("Cannot allocate buffer.");
        return -ENOMEM;
    }

    if(copy_from_user(buffer, buf, count) != 0)
    {
        pr_err("copy_from_user return error.");
        kfree(buffer);
        return -EFAULT;
    }
    // For case, when one try to write not null-terminated sequence of bytes,
    // or omit terminated null-character.
    buffer[count] = '\0';

    /*
     * Usually, writting to the control file is performed via 'echo' command,
     * which append new-line symbol to the writting string.
     *
     * Because, this symbol is usually not needed, we trim it.
     */
    if(buffer[count - 1] == '\n') buffer[count - 1] = '\0';

    *out_str = buffer;
    return count;
}
