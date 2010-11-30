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

#include "kedr_stack_trace.h"

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
 * Areas in the memory image of the target module (used to output 
 * addresses and offsets of the calls made by the module)
 *********************************************************************/
/* Start address and size of "core" area: .text, etc. */
static void *target_core_addr = NULL;
static unsigned int target_core_size = 0;

/* Start address and size of "init" area: .init.text, etc. */
static void *target_init_addr = NULL;
static unsigned int target_init_size = 0;

/*********************************************************************
 * The callbacks to be called after the target module has just been
 * loaded and, respectively, when it is about to unload.
 *********************************************************************/
static void
target_load_callback(struct module *target_module)
{
    BUG_ON(target_module == NULL);

    target_core_addr = target_module->module_core;
    target_core_size = target_module->core_text_size;

    target_init_addr = target_module->module_init;
    target_init_size = target_module->init_text_size;
    return;
}

static void
target_unload_callback(struct module *target_module)
{
    BUG_ON(target_module == NULL);
    
    target_core_addr = NULL;
    target_core_size = 0;

    target_init_addr = NULL;
    target_init_size = 0;
    return;
}

/*********************************************************************
 * Replacement functions
 *********************************************************************/

static void*
repl___kmalloc(size_t size, gfp_t flags)
{
    void* returnValue;
    
    // TODO
    
    /* Call the target function */
    returnValue = __kmalloc(size, flags);
    
    return returnValue;
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
    
    KEDR_MSG("[kedr_leak_check] Cleanup complete\n");
    return;
}

static int __init
payload_init_module(void)
{
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
    
    return kedr_payload_register(&payload);
}

module_init(payload_init_module);
module_exit(payload_cleanup_module);
/*********************************************************************/
