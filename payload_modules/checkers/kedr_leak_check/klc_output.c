/* klc_output.h
 * Helpers for data output.
 * This provides additional abstraction that allows to output data from 
 * the payload module without directly using printk, trace-related stuff
 * or whatever. The way the data is output is subject to change, this 
 * abstraction helps make these changes local.
 */

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

#include <linux/kernel.h>
#include <linux/slab.h>

#include "klc_output.h"

void
klc_print_string(enum klc_output_type output_type, const char *s)
{
    BUG_ON(s == NULL);
    // TODO: BUG_ON(output resources are not initialized)

    switch (output_type) {
    case KLC_UNALLOCATED_FREE: 
    case KLC_UNFREED_ALLOC:
    case KLC_OTHER:
        printk(KERN_INFO "[kedr_leak_check] %s\n", s);
        break;
    default:
        printk(KERN_WARNING "[kedr_leak_check] unknown output type: %d\n", 
            (int)output_type);
        break;
    }
    return;
}

void
klc_print_stack_trace(enum klc_output_type output_type, 
    unsigned long *stack_entries, unsigned int num_entries)
{
    static const char* fmt = "[<%p>] %pS";
    
    /* This is just to pass a buffer of known size to the first call
     * to snprintf() to determine the length of the string to which
     * the data will be converted. 
     */
    char one_char[1];
    char *buf = NULL;
    int len;
    unsigned int i;
    
    BUG_ON(stack_entries == NULL);
    
    if (num_entries == 0)
        return;
    
    for (i = 0; i < num_entries; ++i) {
        len = snprintf(&one_char[0], 1, fmt, 
            (void *)stack_entries[i], (void *)stack_entries[i]);
        buf = (char*)kmalloc(len + 1, GFP_KERNEL);
        if (buf != NULL) {
            snprintf(buf, len + 1, fmt, 
                (void *)stack_entries[i], (void *)stack_entries[i]);
            klc_print_string(output_type, buf);
            kfree(buf);
        } else { 
            printk(KERN_ERR "[kedr_leak_check] klc_print_stack_trace: "
                "not enough memory to prepare a message of size %d\n",
                len);
        }
    }
    return;    
}

void
klc_print_target_module_info(struct module *target_module)
{
    static const char* fmt = 
"Target module: \"%s\", init area at 0x%p, core area at 0x%p";
    
    char one_char[1];
    char *buf = NULL;
    int len;
    const char *name;
    
    BUG_ON(target_module == NULL);
    name = module_name(target_module);
    
    len = snprintf(&one_char[0], 1, fmt, name, 
        target_module->module_init, target_module->module_core);
    buf = (char*)kmalloc(len + 1, GFP_KERNEL);
    if (buf == NULL) {
        printk(KERN_ERR "[kedr_leak_check] klc_print_target_module_info: "
            "not enough memory to prepare a message of size %d\n",
            len);
    }
    snprintf(buf, len + 1, fmt, name, 
        target_module->module_init, target_module->module_core);
    klc_print_string(KLC_OTHER, buf);
    kfree(buf);
    return;
}

void 
klc_print_alloc_info(struct klc_memblock_info *alloc_info)
{
    static const char* fmt = 
        "Block at 0x%p, size: %zu; stack trace of the allocation:";
    
    char one_char[1];
    char *buf = NULL;
    int len;
    
    BUG_ON(alloc_info == NULL);
    
    len = snprintf(&one_char[0], 1, fmt, alloc_info->block, 
        alloc_info->size);
    buf = (char*)kmalloc(len + 1, GFP_KERNEL);
    if (buf == NULL) {
        printk(KERN_ERR "[kedr_leak_check] klc_print_alloc_info: "
            "not enough memory to prepare a message of size %d\n",
            len);
    }
    snprintf(buf, len + 1, fmt, alloc_info->block, alloc_info->size);
    klc_print_string(KLC_UNFREED_ALLOC, buf);
    kfree(buf);
    
    klc_print_stack_trace(KLC_UNFREED_ALLOC, 
        &(alloc_info->stack_entries[0]), alloc_info->num_entries);
    return;
}

void 
klc_print_dealloc_info(struct klc_memblock_info *dealloc_info)
{
    static const char* fmt = 
        "Block at 0x%p; stack trace of the deallocation:";
    
    char one_char[1];
    char *buf = NULL;
    int len;
 
    BUG_ON(dealloc_info == NULL);
    
    len = snprintf(&one_char[0], 1, fmt, dealloc_info->block);
    buf = (char*)kmalloc(len + 1, GFP_KERNEL);
    if (buf == NULL) {
        printk(KERN_ERR "[kedr_leak_check] klc_print_dealloc_info: "
            "not enough memory to prepare a message of size %d\n",
            len);
    }
    snprintf(buf, len + 1, fmt, dealloc_info->block);
    klc_print_string(KLC_UNALLOCATED_FREE, buf);
    kfree(buf);
    
    klc_print_stack_trace(KLC_UNALLOCATED_FREE, 
        &(dealloc_info->stack_entries[0]), dealloc_info->num_entries);
    return;
}

int 
klc_output_init(void)
{
    // TODO
    return 0;
}

void
klc_output_clear(void)
{
    // TODO
    return;
}

void
klc_output_fini(void)
{
    // TODO
    return;
}
/* ================================================================ */
