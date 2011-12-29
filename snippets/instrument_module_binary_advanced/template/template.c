#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "payload.h"

MODULE_AUTHOR("Tsyvarev Andrey");
MODULE_LICENSE("GPL");

struct template_replacement
{
    /*
     *  Instead of 'orig' function target should call 'intermediate'
     * one, which in turns calls 'repl'.
     * 
     * 'orig' function is imported from somewhere, 'repl' is imported
     * from payload module, intermediate should be instantiated using
     * template.
     */
    void* orig;
    void* repl;
    void* intermediate;
    // For alignment purposes
    void* unused;
};


void* __kmalloc_intermediate(size_t size, gfp_t flags)
__attribute__((section(".replacements.text")));
void* __kmalloc_intermediate(size_t size, gfp_t flags)
{
    struct call_info info;
    info.m = THIS_MODULE;
    return __kmalloc_repl(size, flags, &info);
}

struct template_replacement __kmalloc_replacement
__attribute__((section(".replacements"))) =
{
    .orig = &__kmalloc,
    .repl = &__kmalloc_repl,
    .intermediate = &__kmalloc_intermediate
};

void kfree_intermediate(void* p)
__attribute__((section(".replacements.text")));
void kfree_intermediate(void* p)
{
    struct call_info info;
    info.m = THIS_MODULE;
    kfree_repl(p, &info);
}

struct template_replacement kfree_replacement
__attribute__((section(".replacements"))) =
{
    .orig = &kfree,
    .repl = &kfree_repl,
    .intermediate = &kfree_intermediate
};

int init_module_dummy(void) {return 0;}

int init_module_intermediate(void)
__attribute__((section(".init.replacements.text")));
int init_module_intermediate(void)
{
    return payload_init_target(THIS_MODULE, &init_module_dummy);
}

void exit_module_dummy(void) {return;}

void exit_module_intermediate(void)
__attribute__((section(".exit.replacements.text")));
void exit_module_intermediate(void)
{
    payload_exit_target(THIS_MODULE, &exit_module_dummy);
}


static int __init
template_init(void)
{
    return 0;
}

static void __exit
template_exit(void)
{
    return;
}

module_init(template_init);
module_exit(template_exit);