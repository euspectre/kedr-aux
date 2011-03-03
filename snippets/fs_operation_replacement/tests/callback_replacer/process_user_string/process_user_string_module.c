#include "process_user_string.h"
#include "usp_replacer.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/uaccess.h>

static int process_user_string_impl(const char __user* buf, size_t count,
    user_string_processor usp, void *data)
{
    char* str = kmalloc(count + 1, GFP_KERNEL);
    if(str == NULL)
    {
        pr_err("Failed to allocate buffer for user input.");
        return -ENOMEM;
    }
    if(copy_from_user(str, buf, count))
    {
        kfree(str);
        return -EFAULT;
    }
    
    str[count] = '\0';
    
    usp(str, data);
    
    kfree(str);
    
    return 0;
}

int process_user_string(const char __user* buf, size_t count,
    user_string_processor usp, void *data)
{
    int result;
    user_string_processor usp__repl;
    
    usp_target_load_callback(THIS_MODULE);
    
    usp__repl = usp_replace(usp, data);
    
    result = process_user_string_impl(buf, count, usp__repl, data);
    
    usp_replacement_clean(data);
    
    usp_target_unload_callback(THIS_MODULE);
    
    return result;
}
EXPORT_SYMBOL(process_user_string);

static int __init
this_module_init(void)
{
    return 0;
}

static void __exit
this_module_exit(void)
{
}

module_init(this_module_init);
module_exit(this_module_exit);