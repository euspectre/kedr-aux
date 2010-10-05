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
 * Every moment there is at most one generation instance, which export via file
 * name of its object.
 *
 * Name of this file is same as name of generation itself(dynamic control file).
 *
 * New generation is created via another control file(static control file).
 */

struct generation
{
    char* generation_name;
    //dynamic control file
    struct dentry* generation_file;
    
    char* object_name;
};
static struct generation* generation = NULL;
//this name is mean that there is no generation currently.
static const char* generation_name_absent = "null";
//similar for object name
static const char* object_name_default = "noname";
//Protect generation and its object from concurrent read and write.
DEFINE_MUTEX(generation_mutex);



/*
 * Directory which will contatins our files(in our case - 1 file).
 */
struct dentry* module_dir;


// Static control file.
struct dentry* generation_file;

///////////////////Auxiliary functions////////////////////////////////
// These all 5 functions should be executed under spinlock taken
static struct generation* generation_get_internal(void);
static void generation_destroy_internal(void);
static int generation_create_internal(const char* generation_name);

static const char* get_object_name_internal(struct generation* g);
static int set_object_name_internal(struct generation* g, const char* new_name);

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
generation_file_llseek(struct file *filp, loff_t off, int whence)
{
    loff_t result;
    struct generation* g;

    if(mutex_lock_killable(&generation_mutex))
    {
        pr_debug("llseek() operation was killed.");
        return -EAGAIN;
    }
    g = generation_get_internal();
    result = string_operation_llseek(g ? g->generation_name : generation_name_absent,
        &filp->f_pos, off, whence);
        
    mutex_unlock(&generation_mutex);
    
    return result;
}

/*
 * Get current characteristic of the module.
 *
 * In our case - return name of object.
 */
static ssize_t 
generation_file_read(struct file *filp, char __user *buf, size_t count, 
	loff_t *f_pos)
{
    ssize_t result;
    struct generation* g;
    
    if(mutex_lock_killable(&generation_mutex))
    {
        pr_debug("read() operation was killed.");
        return -EAGAIN;
    }
    
    g = generation_get_internal();
    result = string_operation_read(g ? g->generation_name : generation_name_absent,
        buf, count, f_pos);
        
    mutex_unlock(&generation_mutex);
    
    return result;
}

/*
 *
 */
static ssize_t 
generation_file_write(struct file *filp, const char __user *buf,
    size_t count, loff_t *f_pos)
{
    char* write_str;

    ssize_t result = string_operation_write(&write_str, buf, count, f_pos);
    
    if(result < 0) return result;//error

    if(mutex_lock_killable(&generation_mutex))
    {
        pr_debug("write() operation was killed.");
        kfree(write_str);
        return -EAGAIN;
    }
    if(strcmp(write_str, generation_name_absent) == 0)
    {
        generation_destroy_internal();
        result = 0;
    }
    else
    {
        result = generation_create_internal(write_str);
    }
    
    mutex_unlock(&generation_mutex);

    kfree(write_str);
    return result ? result : count;
}

static struct file_operations generation_file_operations =
{
    .owner = THIS_MODULE,//prevent module from unload while file is opened.
    .llseek = generation_file_llseek, //size
    .read = generation_file_read, //get name of current generation
    .write = generation_file_write, //create new generation instead current
};
///////////////Implementation of dynamic control file operations

/*
 * Each operation, which need generation object(this object is create this file),
 * should get this object from private data of inode.
 *
 * NULL pointer in this field means that generation object is already destroyed,
 * so file is meaningless.
 *
 * Note, that only direct reference to these private data should be used, without cache in 'filp'.
 */

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
    struct generation* g;

    if(mutex_lock_killable(&generation_mutex))
    {
        pr_debug("llseek() operation was killed.");
        return -EAGAIN;
    }

    g = filp->f_dentry->d_inode->i_private;
    if(g)
        result = string_operation_llseek(get_object_name_internal(g), &filp->f_pos, off, whence);
    else
        result = -EINVAL;// really, file should be already destroyed
        
    mutex_unlock(&generation_mutex);
    
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
    struct generation* g;
    
    if(mutex_lock_killable(&generation_mutex))
    {
        pr_debug("read() operation was killed.");
        return -EAGAIN;
    }

    g = filp->f_dentry->d_inode->i_private;
    if(g)
        result = string_operation_read(get_object_name_internal(g), buf, count, f_pos);
    else
        result = -EINVAL;// really, file should be already destroyed
        
    mutex_unlock(&generation_mutex);
    
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
    struct generation* g;

    ssize_t result = string_operation_write(&write_str, buf, count, f_pos);
    
    if(result < 0) return result;//error

    if(mutex_lock_killable(&generation_mutex))
    {
        pr_debug("write() operation was killed.");
        kfree(write_str);
        return -EAGAIN;
    }
    g = filp->f_dentry->d_inode->i_private;
    if(g)
        result = set_object_name_internal(g, write_str);
    else
        result = -EINVAL;// really, file should be already destroyed
    
    mutex_unlock(&generation_mutex);

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
    module_dir = debugfs_create_dir("dynamic_control_files_example", NULL);
    if(module_dir == NULL)
    {
        pr_err("Cannot create root directory in debugfs for service.");
        return -1;
    }
    generation_file = debugfs_create_file("current_generation",
        S_IRUGO | S_IWUSR | S_IWGRP,
        module_dir,
        NULL,
        &generation_file_operations
        );
    if(generation_file == NULL)
    {
        pr_err("Cannot create static control file.");
        debugfs_remove(module_dir);
        return -1;
    }
    
    return 0;
}

static void
this_module_exit(void)
{
    //no concurrent usage of generation file(otherwise we wouldn't be here)
    //so no locks
    generation_destroy_internal();
    debugfs_remove(generation_file);
    debugfs_remove(module_dir);

}
module_init(this_module_init);
module_exit(this_module_exit);

////////////////////Implementation of auxiliary functions////////////////////////////////

struct generation* generation_get_internal(void)
{
    return generation;
}
static void generation_destroy_internal(void)
{
    if(generation)
    {
        //key step - before deleting file, mark all opening instance of it invalide
        // that is, each operations with this instance do nothing (or return error)
        generation->generation_file->d_inode->i_private = NULL;
        debugfs_remove(generation->generation_file);
        kfree(generation->object_name);
        kfree(generation->generation_name);
        kfree(generation);
        generation = NULL;
    }
}
static int generation_create_internal(const char* generation_name)
{
    struct generation* g;
    
    generation_destroy_internal();
    
    g = kmalloc(sizeof(*g), GFP_KERNEL);
    if(g == NULL)
    {
        pr_err("Cannot allocate generation struct.");
        return -ENOMEM;
    }
    g->generation_name = kmalloc(strlen(generation_name) + 1, GFP_KERNEL);
    if(g->generation_name == NULL)
    {
        pr_err("Cannot allocate name of generation.");
        kfree(g);
        return -ENOMEM;
    }
    strcpy(g->generation_name, generation_name);

    g->object_name = kmalloc(strlen(object_name_default) + 1, GFP_KERNEL);
    if(g->object_name == NULL)
    {
        pr_err("Cannot allocate name of object.");
        kfree(g->generation_name);
        kfree(g);
        return -ENOMEM;
    }
    strcpy(g->object_name, object_name_default);
    
    g->generation_file = debugfs_create_file(generation_name,
        S_IRUGO | S_IWUSR | S_IWGRP,
        module_dir,
        g,
        &object_name_file_operations);
    
    if(g->generation_file == NULL)
    {
        pr_err("Cannot create dynamic control file for generation.");
        kfree(g->object_name);
        kfree(g->generation_name);
        kfree(g);
        return -EINVAL;
    }
    
    generation = g;

    return 0;
}

static const char* get_object_name_internal(struct generation* g)
{
    return g->object_name;
}
static int set_object_name_internal(struct generation* g, const char* new_name)
{
    char* name = kmalloc(strlen(new_name) + 1, GFP_KERNEL);
    if(name == NULL)
    {
        pr_err("Cannot allocate memory for object name.");
        return -ENOMEM;
    }
    strcpy(name, new_name);
    
    kfree(g->object_name);

    g->object_name = name;
    
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
