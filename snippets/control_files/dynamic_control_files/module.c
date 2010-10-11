#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */

#include <linux/module.h>
    
#include <linux/mutex.h>

#include <linux/debugfs.h> /* control file will be create on debugfs*/

#include <linux/string.h> /* strlen, strcpy */

#include "control_file.h"

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
//Static control file
char* generation_file_get_str(struct inode* inode);
int generation_file_set_str(const char* str, struct inode* inode);

CONTROL_FILE_OPS(generation_file_operations, generation_file_get_str, generation_file_set_str);

//Dynamic control files
char* object_name_file_get_str(struct inode* inode);
int object_name_file_set_str(const char* str, struct inode* inode);

CONTROL_FILE_OPS(object_name_file_operations, object_name_file_get_str, object_name_file_set_str);
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

//Static control file
char* generation_file_get_str(struct inode* inode)
{
    char* str;
    struct generation* g;
    
    if(mutex_lock_killable(&generation_mutex))
    {
        pr_debug("operation was killed.");
        return NULL;
    }
    
    g = generation_get_internal();
    str = kstrdup(g ? g->generation_name : generation_name_absent, GFP_KERNEL);
        
    mutex_unlock(&generation_mutex);
    
    return str;
}
int generation_file_set_str(const char* str, struct inode* inode)
{
    int result;
    if(mutex_lock_killable(&generation_mutex))
    {
        pr_debug("operation was killed.");
        return -EAGAIN;
    }
    if(strcmp(str, generation_name_absent) == 0)
    {
        generation_destroy_internal();
        result = 0;
    }
    else
    {
        result = generation_create_internal(str);
    }
    
    mutex_unlock(&generation_mutex);

    return result;
}

//Dynamic control files

/*
 * Each operation, which need generation object(this object is create this file),
 * should get this object from private data of inode.
 *
 * NULL pointer in this field means that generation object is already destroyed,
 * so file is meaningless.
 *
 * Note, that only direct reference to these private data should be used, without cache in 'filp'.
 */

char* object_name_file_get_str(struct inode* inode)
{
    char* str;
    struct generation* g;

    if(mutex_lock_killable(&generation_mutex))
    {
        pr_debug("operation was killed.");
        return NULL;
    }

    g = inode->i_private;
    if(g)
        str = kstrdup(get_object_name_internal(g), GFP_KERNEL);
    else
        str = NULL;// really, file should be already destroyed
        
    mutex_unlock(&generation_mutex);
    
    return str;
}
int object_name_file_set_str(const char* str, struct inode* inode)
{
    struct generation* g;
    int result;
    
    if(mutex_lock_killable(&generation_mutex))
    {
        pr_debug("write() operation was killed.");
        return -EAGAIN;
    }
    g = inode->i_private;
    if(g)
        result = set_object_name_internal(g, str);
    else
        result = -EINVAL;// really, file should be already destroyed
    
    mutex_unlock(&generation_mutex);

    return result;
}
/////////////////////////////////////////////////////////////

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
