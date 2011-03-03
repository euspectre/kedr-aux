#include "delegate.h"
#include "delegate_operation_replacer.h"

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
int do_something__repl(struct delegate_object* obj);

struct delegate_operations repl =
{
    .do_something = do_something__repl,
};

struct delegate_operations mask =
{
    .do_something = REPLACEMENT_MASK,
};

struct operation_payload payload =
{
    .m = THIS_MODULE,
    .repl = &repl,
    .mask = &mask,
};

int do_something__repl(struct delegate_object* obj)
{
    typeof(do_something__repl)* do_something__orig =
#ifdef PAYLOAD_SPECIAL
        delegate_operations_get_orig_special(do_something, obj, &payload);
#else
        delegate_operations_get_orig(do_something, obj);
#endif
    was_intercepted = 1;
    
    return do_something__orig(obj);
}


static int __init
payload_init(void)
{
    int result =
#ifdef PAYLOAD_SPECIAL
        delegate_operations_payload_register_special(&payload);
#else
        delegate_operations_payload_register(&payload);
#endif
    if(result) return result;
    
    return 0;
}

static void __exit
payload_exit(void)
{
#ifdef PAYLOAD_SPECIAL
    delegate_operations_payload_unregister_special(&payload);
#else
    delegate_operations_payload_unregister(&payload);
#endif
}

module_init(payload_init);
module_exit(payload_exit);