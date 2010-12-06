#include <linux/slab.h>

#include <kedr/base/common.h>
#include "memory_pool.h"

#include <linux/module.h>
#include <linux/kernel.h>

static mempool_allocator_t allocator;

static void* 
ops_alloc(size_t size, void* data)
{
    return kmalloc(size, *((gfp_t*)data));
}

static void
ops_free(void* addr)
{
    kfree(addr);
}


struct mempool_allocator_ops ops = 
{
    .alloc = ops_alloc,
    .free = ops_free
};

static void*
repl___kmalloc(size_t size, gfp_t flags)
{
    return mempool_allocator_alloc(allocator, size, &flags);
}

static void*
repl_kmem_cache_alloc(struct kmem_cache* mc, gfp_t flags)
{
    return mempool_allocator_alloc(allocator,
        kmem_cache_size(mc), &flags);
}

static unsigned long
repl___get_free_pages(gfp_t flags, unsigned int order)
{
    return (unsigned long)mempool_allocator_alloc(allocator,
        PAGE_SIZE << order, &flags);
}


static void
repl_kfree(void* addr)
{
    if(mempool_allocator_free(allocator, addr))
        kfree(addr);
}

static void
repl_kmem_cache_free(struct kmem_cache* mc, void* addr)
{
    if(mempool_allocator_free(allocator, addr))
        kmem_cache_free(mc, addr);
}

static void
repl_free_pages(unsigned long addr, unsigned int order)
{
    if(mempool_allocator_free(allocator, (void*)addr))
        free_pages(addr, order);
}

/* Names and addresses of the functions of interest */
static void* orig_addrs[] = {
    (void*)&__kmalloc,
    (void*)&kfree,
    (void*)&kmem_cache_alloc,
    (void*)&kmem_cache_free,
    (void*)&__get_free_pages,
    (void*)&free_pages
};

/* Addresses of the replacement functions - must go in the same order 
 * as for the original functions.
 */
static void* repl_addrs[] = {
    (void*)&repl___kmalloc,
    (void*)&repl_kfree,
    (void*)&repl_kmem_cache_alloc,
    (void*)&repl_kmem_cache_free,
    (void*)&repl___get_free_pages,
    (void*)&repl_free_pages
};

static struct kedr_payload payload = {
    .mod                    = THIS_MODULE,
    .repl_table.orig_addrs  = orig_addrs,
    .repl_table.repl_addrs  = repl_addrs,
    .repl_table.num_addrs   = ARRAY_SIZE(orig_addrs),
    .target_load_callback   = NULL,
    .target_unload_callback = NULL
};

static int __init
this_module_init(void)
{
    int result;
    allocator = mempool_allocator_create(&ops);
    if(allocator == 0)
    {
        pr_err("Cannot create allocator.");
        return -ENOMEM;
    }
    result = kedr_payload_register(&payload);
    if(result)
    {
        pr_err("Cannot register payload.");
        mempool_allocator_destroy(allocator);
        return result;
    }
    return 0;
}

static void __exit
this_module_exit(void)
{
    kedr_payload_unregister(&payload);
    mempool_allocator_destroy(allocator);
}

module_init(this_module_init);
module_exit(this_module_exit);

MODULE_AUTHOR("Tsyvarev");
MODULE_LICENSE("GPL");