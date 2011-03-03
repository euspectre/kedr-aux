#include "delegate_operation_replacer.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_AUTHOR("Tsyvarev Andrey");
MODULE_LICENSE("GPL");

static operation_replacer delegate_operations_replacer;

int delegate_operations_payload_register(struct operation_payload* payload)
{
    return operation_payload_register(delegate_operations_replacer, payload);
}
EXPORT_SYMBOL(delegate_operations_payload_register);

int delegate_operations_payload_unregister(struct operation_payload* payload)
{
    return operation_payload_unregister(delegate_operations_replacer, payload);
}
EXPORT_SYMBOL(delegate_operations_payload_unregister);

int delegate_operations_payload_register_special(struct operation_payload* payload)
{
    return operation_payload_register_special(delegate_operations_replacer, payload);
}
EXPORT_SYMBOL(delegate_operations_payload_register_special);

int delegate_operations_payload_unregister_special(struct operation_payload* payload)
{
    return operation_payload_unregister_special(delegate_operations_replacer, payload);
}
EXPORT_SYMBOL(delegate_operations_payload_unregister_special);

void delegate_operations_target_load_callback(struct module* m)
{
    operation_target_load_callback(delegate_operations_replacer, m);
}
EXPORT_SYMBOL(delegate_operations_target_load_callback);

void delegate_operations_target_unload_callback(struct module* m)
{
    operation_target_unload_callback(delegate_operations_replacer, m, NULL);
}
EXPORT_SYMBOL(delegate_operations_target_unload_callback);

int delegate_operations_replace(struct delegate_object* obj)
{
    return operation_replace(delegate_operations_replacer, &obj->ops);
}
EXPORT_SYMBOL(delegate_operations_replace);

int delegate_operations_restore(struct delegate_object* obj)
{
    return operation_restore(delegate_operations_replacer, &obj->ops);
}
EXPORT_SYMBOL(delegate_operations_restore);

int delegate_operations_replacement_update(struct delegate_object* obj)
{
    return operation_replacement_update(delegate_operations_replacer, &obj->ops);
}
EXPORT_SYMBOL(delegate_operations_replacement_update);

void* delegate_operations_get_orig_f(int op_offset, struct delegate_object* obj)
{
    return operation_get_orig(delegate_operations_replacer, op_offset, &obj->ops);
}
EXPORT_SYMBOL(delegate_operations_get_orig_f);

void* delegate_operations_get_orig_special_f(int op_offset, struct delegate_object* obj,
    struct operation_payload* payload)
{
    return operation_get_orig_special(delegate_operations_replacer, op_offset, &obj->ops,
        payload);
}
EXPORT_SYMBOL(delegate_operations_get_orig_special_f);

static int __init
delegate_operations_replacer_init(void)
{
    delegate_operations_replacer = operation_replacer_create(10,
        sizeof(struct delegate_operations));
    if(delegate_operations_replacer == NULL)
    {
        pr_err("Fail to create replacer for delegate operations.");
        return -ENOMEM;
    }
    return 0;
}
static void __exit
delegate_operations_replacer_exit(void)
{
    operation_replacer_destroy(delegate_operations_replacer);
}

module_init(delegate_operations_replacer_init);
module_exit(delegate_operations_replacer_exit);