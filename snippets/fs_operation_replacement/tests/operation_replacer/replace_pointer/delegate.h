#ifndef DELEGATE_H
#define DELEGATE_H

struct delegate_operations;
struct delegate_object
{
    const char* name;
    const struct delegate_operations* ops;
};

struct delegate_operations
{
    int (*do_something)(struct delegate_object* obj);
};

int delegate_register(struct delegate_object* obj);
int delegate_unregister(struct delegate_object* obj);

//For detect errors in module implementing delegate
void report_error(int error);

#endif