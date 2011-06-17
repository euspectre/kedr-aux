/*
 * Allow to call certain memory allocation functions from user space.
 */

/* ========================================================================
 * Copyright (C) 2010-2011, Institute for System Programming 
 *                          of the Russian Academy of Sciences (ISPRAS)
 * Authors: 
 *      Eugene A. Shatokhin <spectre@ispras.ru>
 *      Andrey V. Tsyvarev  <tsyvarev@ispras.ru>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 ======================================================================== */

#include <linux/kernel.h>	/* printk() */

#include <linux/module.h>

#include <linux/slab.h> /*kmalloc*/
#include <linux/uaccess.h> /* copy_from_user */
    
#include <linux/debugfs.h> /* control files will be create on debugfs*/
#include <linux/mutex.h> /* mutex for 'last_allocated'*/

MODULE_AUTHOR("Tsyvarev");
MODULE_LICENSE("GPL");

/*
 * Directory which will contatins control file.
 */
struct dentry* module_dir;

struct dentry* kmalloc_file;

struct dentry* last_allocated_file;

void* last_allocated = NULL;
DEFINE_MUTEX(last_allocated_mutex);

/*
 * Push address as 'last_allocated'.
 *
 * Return 0 on success.
 * Return negative error code on fail.
 * 
 * If 'last_allocated' has not pop after last pushing, return -EBUSY.
 */
static int push_last_allocated(void* addr);

/*
 * Return 'last_allocated' and clear it.
 * 
 * If 'last_allocated' hasn not set or already has read, return NULL.
 * 
 * On error return ERR_PTR().
 */
static void* pop_last_allocated(void);

///////////////////////////File operations///////////////////////
static ssize_t
kmalloc_file_write(struct file* filp,
    const char __user *buf, size_t count, loff_t* f_pos)
{
    void* alloc_result = NULL;

    char* str_param = kmalloc(count + 1, GFP_KERNEL);
    unsigned long param;
    if(str_param == NULL)
    {
        pr_err("[memory_allocator]Failed to allocate memory for parameter of kmalloc().");
        return -ENOMEM;
    }
    
    if(copy_from_user(str_param, buf, count))
    {
        kfree(str_param);
        return -EFAULT;
    }
    
    str_param[count] = '\0';
    
    if(strict_strtoul(str_param, 0, &param))
    {
        pr_err("Fail to parse '%s' as interger.", str_param);
        kfree(str_param);
        return -EINVAL;
    }
    
    kfree(str_param);
    
    alloc_result = kmalloc((size_t)param, GFP_KERNEL);
    if(alloc_result)
    {
        int error = push_last_allocated(alloc_result);
        if(error)
        {
            kfree(alloc_result);
            return error;
        }
    }
    else
    {
        return -ENOMEM;
    }

    return count;
}

static struct file_operations kmalloc_file_operations = {
    .owner = THIS_MODULE,
    .write = kmalloc_file_write
};


static int
last_allocated_file_open(struct inode* inode, struct file* filp)
{
    char* buf;
    size_t buf_size;
    
    int result;
    
    void* last_allocated = pop_last_allocated();
    if(IS_ERR(last_allocated))
    {
        return PTR_ERR(last_allocated);
    }
    
    buf_size = snprintf(NULL, 0, "%p\n", last_allocated) + 1;
    
    buf = kmalloc(buf_size, GFP_KERNEL);
    if(buf == NULL)
    {
        pr_err("Failed to allocate buffer for 'last_allocated' value.");
        return -ENOMEM;
    }
    
    snprintf(buf, buf_size, "%p\n", last_allocated);
    
    filp->private_data = buf;
    
    result = nonseekable_open(inode, filp);
    
    if(result) kfree(buf);
    
    return result;
}

static int
last_allocated_file_release(struct inode* inode, struct file* filp)
{
    kfree(filp->private_data);
    
    return 0;
}

static ssize_t
last_allocated_file_read(struct file* filp, char __user* buf,
    size_t count, loff_t *f_pos)
{
    size_t dataLen;
    loff_t pos = *f_pos;
    const char *data = (const char *)filp->private_data;
    
    if (data == NULL) return -EINVAL;
    dataLen = strlen(data) + 1;
    
    /* Reading outside of the data buffer is not allowed */
    if ((pos < 0) || (pos > dataLen)) return -EINVAL;
    
    /* EOF reached or 0 bytes requested */
    if ((count == 0) || (pos == dataLen)) return 0;
    
    if (pos + count > dataLen) count = dataLen - pos;
    if (copy_to_user(buf, &data[pos], count) != 0)
        return -EFAULT;
    
    *f_pos += count;
    return count;
}

static struct file_operations last_allocated_file_operations = {
    .owner = THIS_MODULE,
    .open = last_allocated_file_open,
    .release = last_allocated_file_release,
    .read = last_allocated_file_read,
};
///
static int __init
this_module_init(void)
{
    int err;
    
    module_dir = debugfs_create_dir("kernel_allocator", NULL);
    if(module_dir == NULL)
    {
        pr_err("Cannot create root directory in debugfs for module.");
        return -EINVAL;
    }

    kmalloc_file = debugfs_create_file("kmalloc",
        S_IWUSR,
        module_dir,
        NULL,
        &kmalloc_file_operations
        );
    if(kmalloc_file == NULL)
    {
        pr_err("Cannot create file for last_allocated.");
        
        err = -EINVAL;
        goto err_kmalloc_file;
    }

    last_allocated_file = debugfs_create_file("last_allocated",
        S_IWUSR,
        module_dir,
        NULL,
        &last_allocated_file_operations
        );
    if(last_allocated_file == NULL)
    {
        pr_err("Cannot create file for last_allocated.");
        
        err = -EINVAL;
        goto err_last_allocated_file;
    }


    return 0;

err_last_allocated_file:
    debugfs_remove(kmalloc_file);
err_kmalloc_file:
    debugfs_remove(module_dir);

    return err;
}

static void
this_module_exit(void)
{
    debugfs_remove(last_allocated_file);
    debugfs_remove(kmalloc_file);
    debugfs_remove(module_dir);
}
module_init(this_module_init);
module_exit(this_module_exit);

////////////////////////////////////////////////////////////////
int push_last_allocated(void* addr)
{
    int result;
    int err = mutex_lock_interruptible(&last_allocated_mutex);
    if (err) return err;
    
    if(last_allocated == NULL)
    {
        last_allocated = addr;
        result = 0;
    }
    else
    {
        result = -EBUSY;
    }
    
    mutex_unlock(&last_allocated_mutex);
    
    return result;
}

void* pop_last_allocated(void)
{
    void* result;
    int err = mutex_lock_interruptible(&last_allocated_mutex);
    if(err) return ERR_PTR(err);
    
    result = last_allocated;
    
    mutex_unlock(&last_allocated_mutex);
    
    return result;
}