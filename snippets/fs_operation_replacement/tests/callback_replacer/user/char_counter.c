/*
 * Simply count characters in the user input up to '\0'.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

#include <linux/proc_fs.h> /* file in /proc */
#include <linux/fs.h>

#include "process_user_string.h"

MODULE_AUTHOR("Tsyvarev Andrey");
MODULE_LICENSE("GPL");

static int char_count = 0;
module_param(char_count, int, S_IRUGO | S_IWUSR);

static int was_called = 0;
module_param(was_called, int, S_IRUGO | S_IWUSR);

static void char_counter(const char* str, void* data)
{
    was_called = 1;
    (*(int*) data) = strlen(str);
}

/* 
 * Without syncronization.
 */

struct proc_dir_entry *control_file;
const char* control_file_name = "write_anything_here";

static int
control_file_write(struct file* filp, const char __user *buf,
    size_t count, loff_t* f_pos)
{
    int result;
    int char_count_local = 0;
    result = process_user_string(buf, count, char_counter, &char_count_local);
    if(result) return result;

    char_count += char_count_local;
    return (result < 0) ? result : count;
}
static struct file_operations control_file_ops =
{
    .owner = THIS_MODULE,
    .write = control_file_write,
};

static int __init
char_counter_init(void)
{
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

    return 0;
}

static void __exit
char_counter_exit(void)
{
    remove_proc_entry(control_file_name, NULL);
}

module_init(char_counter_init);
module_exit(char_counter_exit);