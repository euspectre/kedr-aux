#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "usp_replacer.h"

MODULE_AUTHOR("Tsyvare Andrey");
MODULE_LICENSE("GPL");

static callback_replacer usp_replacer;

void usp_target_load_callback(struct module* m)
{
    callback_target_load_callback(usp_replacer, m);
}
EXPORT_SYMBOL(usp_target_load_callback);

static void usp_undeleted_key_callback(void* key)
{
    pr_info("'usp' callback for data %p wasn't cleaned normally.", key);
}
void usp_target_unload_callback(struct module* m)
{
    callback_target_unload_callback(usp_replacer, m,
        usp_undeleted_key_callback);
}
EXPORT_SYMBOL(usp_target_unload_callback);

int usp_payload_register(struct callback_payload* payload)
{
    return callback_payload_register(usp_replacer, payload);
}
EXPORT_SYMBOL(usp_payload_register);

int usp_payload_unregister(struct callback_payload* payload)
{
    return callback_payload_unregister(usp_replacer, payload);
}
EXPORT_SYMBOL(usp_payload_unregister);

user_string_processor
usp_replace(user_string_processor callback, void* data)
{
    return (user_string_processor) callback_replace(usp_replacer, (void*)callback, data);
}
EXPORT_SYMBOL(usp_replace);

int usp_replacement_clean(void* data)
{
    return callback_replacement_clean(usp_replacer, data);
}
EXPORT_SYMBOL(usp_replacement_clean);

user_string_processor
usp_get_orig(void* data)
{
    return (user_string_processor)callback_get_orig(usp_replacer, data);
}
EXPORT_SYMBOL(usp_get_orig);

static int __init
usp_replacer_init(void)
{
    usp_replacer = callback_replacer_create(1);
    if(usp_replacer == NULL)
    {
        pr_err("Cannot create replacer for 'usp' callback.");
        return -ENOMEM;
    }
    return 0;
}

static void __exit
usp_replacer_exit(void)
{
    callback_replacer_destroy(usp_replacer);
}

module_init(usp_replacer_init);
module_exit(usp_replacer_exit);
