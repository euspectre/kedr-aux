#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "wrapper.h"

MODULE_AUTHOR("Tsyvarev Andrey");
MODULE_LICENSE("GPL");

void* __kMalloc(size_t size, gfp_t flags)
{
    void* result = kmalloc(size, flags);
    pr_info("__kmalloc() is called. Requested size is %uz, flags are 0x%x. "
        "Returned pointer is %p.", size, flags, result);
    return result;
}

EXPORT_SYMBOL(__kMalloc);

void kFree(void* p)
{
    pr_info("kfree() is called. Pointer for free is %p.", p);
    kfree(p);
}

EXPORT_SYMBOL(kFree);

static int __init
wrapper_init(void)
{
    return 0;
}

static void __exit
wrapper_exit(void)
{
    return;
}

declare_replacement(kfree, kFree);
declare_replacement(__kmalloc, __kMalloc);

module_init(wrapper_init);
module_exit(wrapper_exit);