#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <linux/module.h>

struct call_info
{
    struct module* m;
};

int payload_init_target(struct module* m, int (*init_target)(void));
void payload_exit_target(struct module* m, void (*exit_target)(void));
void* __kmalloc_repl(size_t size, gfp_t flags, struct call_info* info);
void kfree_repl(void* p, struct call_info* info);

#endif /* PAYLOAD_H */