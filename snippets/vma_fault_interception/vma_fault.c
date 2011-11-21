/*
 * Record all calls of 'fault' callback operation of vm_area_struct.
 */

#include <linux/module.h>

#include <linux/fs.h>
#include <linux/mm.h>

#include <kedr/core/kedr.h>

MODULE_AUTHOR("Andrey Tsyvarev");
MODULE_LICENSE("GPL");

#include "fs_interception.h"
#include "vma_operations_interceptor.h"
#include "vma_operations_clone_interceptor.h"

/* Intercept 'fault' handler */
static void vma_operations_fault_post_fault(struct vm_area_struct* vma,
    struct vm_fault* vmf, int returnValue,
    struct kedr_coi_operation_call_info* call_info)
{
    // Check if page was successfully mapped
    if(returnValue & (VM_FAULT_ERROR/* | VM_FAULT_RETRY*/)) return;
    
    pr_info("Page at %lu in map [%lx, %lx) was mapped in fault handler.",
        vmf->pgoff - vma->vm_pgoff, vma->vm_start, vma->vm_end);
    
    // Extract page allocated
    if(returnValue & VM_FAULT_NOPAGE)
    {
        /*
         * Callback operation insert one ore more pages into vma.
         * 
         * Because such insertion may be performed via *insert*-like
         * functions, calls of these functions may intercepted in a
         * manner of put_page().
         */
        pr_info("One or more pages was created and inserted into vma "
            "via vm_insert_pfn(%lx) or similar.", vmf->pgoff);
    }
    else
    {
        /*
         * Callback operation allocate page for mapping and let kernel
         * to insert it.
         * 
         * In leak check this moment may be modelled as put_page().
         */
        pr_info("Page %p was created for mapping.", vmf->page);
    }
}

/* Determine mremap call from vma operation */
static void vma_operations_open_post_mremap(
    struct vm_area_struct* vma,
    struct kedr_coi_operation_call_info* call_info)
{
    pr_info("File %p was remapped to [%p, %p) started at offset 0x%zx.",
        vma->vm_file, (void*)vma->vm_start, (void*)vma->vm_end,
        (size_t)(vma->vm_pgoff * PAGE_SIZE));
}


/* Determine lifetime of vma object clone from its operation */
static void vma_operations_open_post_vma_clone_lifetime(
    struct vm_area_struct* vma,
    struct kedr_coi_operation_call_info* call_info)
{
    vma_operations_clone_interceptor_watch(vma, vma->vm_file, &vma->vm_ops);
}


/* Determine lifetime of vma object from its operation */
static void vma_operations_close_post_vma_lifetime(
    struct vm_area_struct* vma,
    struct kedr_coi_operation_call_info* call_info)
{
    vma_operations_interceptor_forget(vma);
    pr_info("Mapping [%p, %p) was closed for file %p.",
        (void*)vma->vm_start, (void*)vma->vm_end, vma->vm_file);
}



/* Determine lifetime of vma object clone from its operation */
static void vma_operations_close_post_vma_clone_lifetime(
    struct vm_area_struct* vma,
    struct kedr_coi_operation_call_info* call_info)
{
    vma_operations_clone_interceptor_forget(vma, &vma->vm_ops);
}


static struct kedr_coi_post_handler vma_operations_post_handlers[] =
{
    vma_operations_open_post_external(vma_operations_open_post_vma_clone_lifetime),
    vma_operations_open_post_external(vma_operations_open_post_mremap),
    vma_operations_fault_post(vma_operations_fault_post_fault),
    vma_operations_close_post_external(vma_operations_close_post_vma_lifetime),
    vma_operations_close_post_external(vma_operations_close_post_vma_clone_lifetime),
    kedr_coi_post_handler_end
};

static struct kedr_coi_payload vma_operations_payload =
{
    .mod = THIS_MODULE,
    .post_handlers = vma_operations_post_handlers
};

/* Determine lifetime of vma object from file operation */
static void file_operations_mmap_post_vma_lifetime(
    struct file* filp, struct vm_area_struct* vma, int returnValue,
    struct kedr_coi_operation_call_info* call_info)
{
    if(returnValue == 0)
    {
        vma_operations_interceptor_watch(vma);
        pr_info("File %p was mapped to [%p, %p) started at offset 0x%zx.",
            filp, (void*)vma->vm_start, (void*)vma->vm_end,
            (size_t)(vma->vm_pgoff * PAGE_SIZE));
    }
}

/* Determine lifetime of vma object cloning from file operation */
static void file_operations_mmap_post_vma_clone_lifetime(
    struct file* filp, struct vm_area_struct* vma, int returnValue,
    struct kedr_coi_operation_call_info* call_info)
{
    if(returnValue == 0)
    {
        vma_operations_clone_interceptor_watch(vma, filp, &vma->vm_ops);
    }
}

static struct kedr_coi_post_handler file_operations_post_handlers[] =
{
    file_operations_mmap_post_external(file_operations_mmap_post_vma_lifetime),
    file_operations_mmap_post_external(file_operations_mmap_post_vma_clone_lifetime),
    kedr_coi_post_handler_end
};

static struct kedr_coi_payload file_operations_payload =
{
    .mod = THIS_MODULE,
    .post_handlers = file_operations_post_handlers
};


/* Define lifetime of file system type(from global functions) */
static void register_filesystem_pre_fst_lifetime(
    struct file_system_type* fs)
{
    file_system_type_interceptor_watch(fs);
}

static void register_filesystem_post_fst_lifetime(
    struct file_system_type* fs, int returnValue)
{
    if(returnValue) //error path
        file_system_type_interceptor_forget(fs);
}

static void unregister_filesystem_post_fst_lifetime(
    struct file_system_type* fs, int returnValue)
{
    if(!returnValue)
        file_system_type_interceptor_forget(fs);
}

static struct kedr_pre_pair pre_pairs[] = 
{
    {
        .orig = register_filesystem,
        .pre = register_filesystem_pre_fst_lifetime
    },
    {
        .orig = NULL
    }
};

static struct kedr_post_pair post_pairs[] = 
{
    {
        .orig = register_filesystem,
        .post = register_filesystem_post_fst_lifetime
    },
    {
        .orig = unregister_filesystem,
        .post = unregister_filesystem_post_fst_lifetime
    },
    {
        .orig = NULL
    }
};


static void on_target_load(struct module* m)
{
    fs_interception_start();
    vma_operations_interceptor_start();
}

static void on_target_unload(struct module* m)
{
    vma_operations_interceptor_stop();
    fs_interception_stop();
}


struct kedr_payload payload =
{
    .mod = THIS_MODULE,
    
    .pre_pairs = pre_pairs,
    .post_pairs = post_pairs,
    
    .target_load_callback = on_target_load,
    .target_unload_callback = on_target_unload
};

/*
 *  These two functions are defined in 'functions_support.c',
 * generated from functions_support.data.
 */
extern int functions_support_register(void);
extern void functions_support_unregister(void);

static int __init vma_fault_interception_init(void)
{
    int result;
    
    result = fs_interception_init();
    if(result) goto err_fs_interception;
    
    result = vma_operations_interceptor_init(NULL);
    if(result) goto err_vma_interception;
    
    result = vma_operations_clone_interceptor_init(
        vma_operations_interceptor_creation_interceptor_create, NULL);
    if(result) goto err_vma_clone_interception;
    //
    result = functions_support_register();
    if(result) goto err_functions_support;

    result = kedr_payload_register(&payload);
    if(result) goto err_payload;
    
    result = file_operations_interceptor_payload_register(&file_operations_payload);
    if(result) goto err_file_payload;
    
    result = vma_operations_interceptor_payload_register(&vma_operations_payload);
    if(result) goto err_vma_payload;

    return 0;

err_vma_payload:
    file_operations_interceptor_payload_unregister(&file_operations_payload);
err_file_payload:
    kedr_payload_unregister(&payload);
err_payload:
    functions_support_unregister();
err_functions_support:
//
    vma_operations_clone_interceptor_destroy();
err_vma_clone_interception:
    vma_operations_interceptor_destroy();
err_vma_interception:
    fs_interception_destroy();
err_fs_interception:

    return result;
}

static void __exit vma_fault_interception_exit(void)
{
    vma_operations_interceptor_payload_unregister(&vma_operations_payload);
    file_operations_interceptor_payload_unregister(&file_operations_payload);
    kedr_payload_unregister(&payload);
    functions_support_unregister();
    //
    vma_operations_clone_interceptor_destroy();
    vma_operations_interceptor_destroy();
    fs_interception_destroy();
}

module_init(vma_fault_interception_init);
module_exit(vma_fault_interception_exit);