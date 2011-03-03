#include "process_user_string.h"
#include "usp_replacer.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

/*
 * Payload for all cases:)
 * 
 * for generate special payload, macro PAYLOAD_SPECIAL should be defined.
 */


MODULE_AUTHOR("Tsyvarev Andrey");
MODULE_LICENSE("GPL");


int was_intercepted;
module_param(was_intercepted, int, S_IRUGO | S_IWUSR);

//Forward declaration for special payload
void user_string_processor__repl(const char* str, void* data);

struct callback_payload payload =
{
    .m = THIS_MODULE,
    .callback = &user_string_processor__repl,
};

void user_string_processor__repl(const char* str, void* data)
{
    typeof(user_string_processor__repl)* usp__orig =
#ifdef PAYLOAD_SPECIAL
        usp_get_orig_special(data, &payload);
#else
        usp_get_orig(data);
#endif
    was_intercepted = 1;
    
    usp__orig(str, data);
}


static int __init
payload_init(void)
{
    int result =
#ifdef PAYLOAD_SPECIAL
        usp_payload_register_special(&payload);
#else
        usp_payload_register(&payload);
#endif
    if(result) return result;
    
    return 0;
}

static void __exit
payload_exit(void)
{
#ifdef PAYLOAD_SPECIAL
    usp_payload_unregister_special(&payload);
#else
    usp_payload_unregister(&payload);
#endif
}

module_init(payload_init);
module_exit(payload_exit);