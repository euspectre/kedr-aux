#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include "delegate.h"

/*
 * Implementation of delegate object.
 * 
 * If UPDATE_OPERATIONS is set,
 * update operations each call of delegate function.
 */

MODULE_AUTHOR("Tsyvarev Andrey");
MODULE_LICENSE("GPL");

static int was_called = 0;
module_param(was_called, int, S_IRUGO | S_IWUSR);

//Forward declaration
static int do_something__impl(struct delegate_object* obj);

static struct delegate_operations ops =
{
    .do_something = do_something__impl,
};

static struct delegate_object obj =
{
    .name = "delegate_impl",
    .ops = &ops,
};

static int do_something__impl(struct delegate_object* obj)
{
#ifdef UPDATE_OPERATIONS
    obj->ops = &ops;
#endif
    was_called = 1;
    return 0;
}


static int __init
delegate_impl_init(void)
{
    int result = delegate_register(&obj);
    if(result) return result;
    
    return 0;
}

static void __exit
delegate_impl_exit(void)
{
    delegate_unregister(&obj);
    if(obj.ops != &ops)
    {
        pr_err("Operations after deregistration differ from original ones.");
        report_error(-EINVAL);
    }
}

module_init(delegate_impl_init);
module_exit(delegate_impl_exit);