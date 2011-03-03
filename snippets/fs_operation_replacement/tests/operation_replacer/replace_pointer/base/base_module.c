#include "delegate.h"
#include "delegate_operation_replacer.h"

#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

/*
 * When compiled with UPDATE_REPLACEMENT macro defined,
 * call operation_replacement_update after delegate function is called.
 */

MODULE_AUTHOR("Tsyvarev Andrey");
MODULE_LICENSE("GPL");

static int status = 0;
module_param(status, int, S_IRUGO);

#include <linux/proc_fs.h> /* file in /proc */
#include <linux/fs.h>

/* 
 * Without syncronization.
 */

struct delegate_object* current_delegate = NULL;

struct proc_dir_entry *control_file;
const char* control_file_name = "delegate";

static int
control_file_write(struct file* filp, const char __user *buf,
    size_t count, loff_t* f_pos)
{
    int result;
    if(current_delegate == NULL)
    {
        pr_err("Delegate is not set.");
        return -EINVAL;
    }
    result = current_delegate->ops->do_something(current_delegate);
#ifdef UPDATE_REPLACEMENT
    delegate_operations_replacement_update(current_delegate);
#endif
    return (result < 0) ? result : count;
}
static struct file_operations control_file_ops =
{
    .owner = THIS_MODULE,
    .write = control_file_write,
};

int delegate_register(struct delegate_object* obj)
{
    if(current_delegate != NULL)
    {
        pr_err("Attempt to register delegate while another delegate is registered.");
        return -EBUSY;
    }
    //Instead of using KEDR for intercept this operation, replace operations here.
    delegate_operations_target_load_callback(THIS_MODULE);
    delegate_operations_replace(obj);
    
    current_delegate = obj;
    
    return 0;
}
EXPORT_SYMBOL(delegate_register);

int delegate_unregister(struct delegate_object* obj)
{
    if(current_delegate != obj)
    {
        pr_err("Attemp to deregister delegate which hasn't registered.");
        return -EINVAL;
    }
    current_delegate = NULL;
    
    delegate_operations_restore(obj);
    delegate_operations_target_unload_callback(THIS_MODULE);
    
    return 0;
}
EXPORT_SYMBOL(delegate_unregister);

void report_error(int error)
{
    if(status == 0) status = error;
}
EXPORT_SYMBOL(report_error);

static int __init
base_module_init(void)
{
    //int result;
    control_file = proc_create_data(control_file_name,
        S_IWUGO,
        NULL,
        &control_file_ops,
        NULL);
    if(control_file == NULL)
    {
        pr_err("Cannot create control file");
        return -EINVAL;
    }
    
    //result = delegate_operations_replacer_init(5);
    //if(result)
    //{
    //    proc_remove_entry(control_file_name, NULL);
    //    return -EINVAL;
    //}
    
    return 0;
}

static void __exit
base_module_exit(void)
{
    remove_proc_entry(control_file_name, NULL);
    //delegate_operations_replacer_destroy();
}

module_init(base_module_init);
module_exit(base_module_exit);