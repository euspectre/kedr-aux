#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/slab.h>

#include "payload.h"

MODULE_AUTHOR("Tsyvarev Andrey");
MODULE_LICENSE("GPL");

int payload_init_target(struct module* m, int (*init_target)(void))
{
    int result;
    pr_info("Module %s is started under payload.", m->name);
    
    result = init_target();
    if(result)
    {
        pr_info("Module %s is stopped.", m->name);
        return result;
    }
    return 0;
}
EXPORT_SYMBOL(payload_init_target);


void payload_exit_target(struct module* m, void (*exit_target)(void))
{
    exit_target();
    pr_info("Module %s is stopped.", m->name);
}
EXPORT_SYMBOL(payload_exit_target);

void* __kmalloc_repl(size_t size, gfp_t flags, struct call_info* info)
{
    void* p = __kmalloc(size, flags);
    pr_info("__kmalloc was called: caller=%s, size=%zu, result=%p",
        info->m->name, size, p);
    return p;
}
EXPORT_SYMBOL(__kmalloc_repl);

void kfree_repl(void* p, struct call_info* info)
{
    pr_info("kfree was called: caller=%s, pointer=%p",
        info->m->name, p);
    kfree(p);
}
EXPORT_SYMBOL(kfree_repl);


static int __init payload_init(void)
{
    pr_info("Payload started.");
    return 0;
}

static void __exit payload_exit(void)
{
    pr_info("Payload stopped.");
    return;
}

module_init(payload_init);
module_exit(payload_exit);