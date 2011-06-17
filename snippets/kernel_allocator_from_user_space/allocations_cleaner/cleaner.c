/*
 * Allow to call certain memory free functions from user space.
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

MODULE_AUTHOR("Tsyvarev");
MODULE_LICENSE("GPL");

/*
 * Directory which will contatins control file.
 */
struct dentry* module_dir;

struct dentry* kfree_file;

///////////////////////////File operations///////////////////////
static ssize_t
kfree_file_write(struct file* filp,
    const char __user *buf, size_t count, loff_t* f_pos)
{
    char* str_param = kmalloc(count + 1, GFP_KERNEL);
    unsigned long param;
    if(str_param == NULL)
    {
        pr_err("[allocations_cleaner]Failed to allocate memory for parameter of kfree().");
        return -ENOMEM;
    }
    
    if(copy_from_user(str_param, buf, count))
    {
        kfree(str_param);
        return -EFAULT;
    }
    
    str_param[count] = '\0';
    
    if(strict_strtoul(str_param, 16, &param))
    {
        pr_err("Fail to parse %s as hexadecimal interger.", str_param);
        kfree(str_param);
        return -EINVAL;
    }
    
    kfree(str_param);
    
    kfree((void*)param);

    return count;
}

static struct file_operations kfree_file_operations = {
    .owner = THIS_MODULE,
    .write = kfree_file_write
};


///
static int __init
this_module_init(void)
{
    module_dir = debugfs_create_dir("allocations_cleaner", NULL);
    if(module_dir == NULL)
    {
        pr_err("Cannot create root directory in debugfs for module.");
        return -EINVAL;
    }

    kfree_file = debugfs_create_file("kfree",
        S_IWUSR,
        module_dir,
        NULL,
        &kfree_file_operations
        );
    if(kfree_file == NULL)
    {
        pr_err("Cannot create trigger file for kfree.");
        debugfs_remove(module_dir);
        return -EINVAL;
    }
    return 0;
}

static void
this_module_exit(void)
{
    debugfs_remove(kfree_file);
    debugfs_remove(module_dir);
}
module_init(this_module_init);
module_exit(this_module_exit);
