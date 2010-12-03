/*********************************************************************
 * Module: kedr_leak_checker
 * TODO: <description>
 *********************************************************************/
/* ========================================================================
 * Copyright (C) 2010, Institute for System Programming 
 *                     of the Russian Academy of Sciences (ISPRAS)
 * Authors: 
 *      Eugene A. Shatokhin <spectre@ispras.ru>
 *      Andrey V. Tsyvarev  <tsyvarev@ispras.ru>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 ======================================================================== */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
/*#include <linux/mutex.h>
#include <linux/spinlock.h>*/
#include <linux/debugfs.h>

#include <kedr/base/common.h>

#include "memblock_info.h"
#include "klc_output.h"
#include "mbi_ops.h"

MODULE_AUTHOR("Eugene A. Shatokhin");
MODULE_LICENSE("GPL");

/*********************************************************************
 * Parameters of the module
 *********************************************************************/
/* Default number of stack frames to output (at most) */
#define KEDR_STACK_DEPTH_DEFAULT 6

/* At most 'max_stack_entries' stack entries will be output for each 
 * suspicious allocation or deallocation. 
 * Should not exceed KEDR_MAX_FRAMES.
 */
unsigned int stack_depth = KEDR_STACK_DEPTH_DEFAULT;
module_param(stack_depth, uint, S_IRUGO);

/*********************************************************************
 * The callbacks to be called after the target module has just been
 * loaded and, respectively, when it is about to unload.
 *********************************************************************/
static void
target_load_callback(struct module *target_module)
{
    BUG_ON(target_module == NULL);
    
    klc_output_clear();
    klc_print_target_module_info(target_module);
    return;
}

static void
target_unload_callback(struct module *target_module)
{
    BUG_ON(target_module == NULL);
    
    klc_flush_allocs();
    klc_flush_deallocs();
    return;
}

/*********************************************************************
 * Replacement functions
 *********************************************************************/

static void*
repl___kmalloc(size_t size, gfp_t flags)
{
    void *ret_val;
   
    /* Call the target function */
    ret_val = __kmalloc(size, flags);
    
    /* Process the allocation */
//<> [DBG]
    {
        /*int found;
        void *ptr;*/
        klc_add_alloc(ret_val, size, stack_depth);
        
        /*ptr = (void *)&repl___kmalloc;
        found = klc_find_and_remove_alloc(ptr);
        printk(KERN_INFO "[DBG] found(0x%p)=%d\n", ptr, found);*/
    }
//<> [/DBG]
    return ret_val;
}

// TODO

/*********************************************************************/

/* Names and addresses of the functions of interest */
static void* orig_addrs[] = {
    (void*)&__kmalloc,
};

/* Addresses of the replacement functions */
static void* repl_addrs[] = {
    (void*)&repl___kmalloc,
};


static struct kedr_payload payload = {
    .mod                    = THIS_MODULE,
    .repl_table.orig_addrs  = &orig_addrs[0],
    .repl_table.repl_addrs  = &repl_addrs[0],
    .repl_table.num_addrs   = ARRAY_SIZE(orig_addrs),
    .target_load_callback   = target_load_callback,
    .target_unload_callback = target_unload_callback
};
/*********************************************************************/

static void
payload_cleanup_module(void)
{
    kedr_payload_unregister(&payload);
    klc_output_fini();
    
    KEDR_MSG("[kedr_leak_check] Cleanup complete\n");
    return;
}

static int __init
payload_init_module(void)
{
    int ret = 0;
    
    BUILD_BUG_ON(ARRAY_SIZE(orig_addrs) != 
        ARRAY_SIZE(repl_addrs));

    KEDR_MSG("[kedr_leak_check] Initializing\n");
    
    if (stack_depth == 0 || stack_depth > KEDR_MAX_FRAMES) {
        printk(KERN_ERR "[kedr_leak_check] "
            "Invalid value of 'stack_depth': %u (should be a positive "
            "integer not greater than %u)\n",
            stack_depth,
            KEDR_MAX_FRAMES
        );
        return -EINVAL;
    }
    
    ret = klc_output_init();
    if (ret != 0)
        return ret;
    
    ret = kedr_payload_register(&payload);
    if (ret != 0) 
        goto fail_reg;
    
// TODO: initialize storage and output subsystems if necessary
   
    return 0;

fail_reg:
    klc_output_fini();
    return ret;
}

module_init(payload_init_module);
module_exit(payload_cleanup_module);
/*********************************************************************/
