#include "operation_replacer.h"
#include "callback_replacer.h"
#include "fs_operation_replacement.h"

#include <linux/kernel.h> /*printk*/
#include <linux/slab.h>  /* kmalloc */
#include <linux/module.h> /* THIS_MODULE */

#include <kedr/base/common.h>

#include <linux/mount.h> /*struct vfsmnt*/

#include "data_map.h" /*map file operations in inode*/

/*
 * Define replacer for file_operations structure.
 */

static operation_replacer file_operations_replacer;
static int file_operations_replacer_init(size_t elem_num)
{
	file_operations_replacer = operation_replacer_create(elem_num,
		sizeof(struct file_operations));
    if(file_operations_replacer == NULL)
    {
        pr_err("file_operations_replacer_init: Cannot create replacer for file operations.");
        return -ENOMEM;
    }
    return 0;
}
static void file_operations_replacer_destroy(void)
{
	operation_replacer_destroy(file_operations_replacer);
}
static void file_operations_target_load_callback(struct module* m)
{
	operation_target_load_callback(file_operations_replacer, m);
}
static void file_operations_undeleted_key_callback(void* key)
{
	pr_info("File operations for file %p hasn't restored normally.",
		container_of(key, struct file, f_op));
}
static void file_operations_target_unload_callback(struct module* m)
{
	operation_target_unload_callback(file_operations_replacer, m,
		file_operations_undeleted_key_callback);
}
static int file_operations_replace(struct file* filp)
{
	return operation_replace(file_operations_replacer, &filp->f_op);
}
static int file_operations_restore(struct file* filp)
{
	return operation_restore(file_operations_replacer, &filp->f_op);
}
// Currently not used
/*static int file_operations_replacement_clean(struct file* filp)
{
	return operation_replacement_clean(file_operations_replacer,
		&filp->f_op);
}*/
static int file_operations_replacement_update(struct file* filp)
{
	return operation_replacement_update(file_operations_replacer,
		&filp->f_op);
}

int file_operations_payload_register(struct operation_payload* payload)
{
	return operation_payload_register(file_operations_replacer,
        payload);
}
int file_operations_payload_unregister(struct operation_payload* payload)
{
	return operation_payload_unregister(file_operations_replacer,
        payload);
}

void* file_operations_get_orig_f(int op_offset, struct file* filp)
{
    return operation_get_orig(file_operations_replacer,
        op_offset, &filp->f_op);
}

int file_operations_payload_register_special(
    struct operation_payload* payload)
{
	return operation_payload_register_special(file_operations_replacer,
		payload);
}
int file_operations_payload_unregister_special(
	struct operation_payload* payload)
{
	return operation_payload_unregister_special(file_operations_replacer,
		payload);
}

void* file_operations_get_orig_special_f(int op_offset,
    struct file* filp, struct operation_payload* payload)
{
    return operation_get_orig_special(file_operations_replacer,
        op_offset, &filp->f_op, payload);
}

/*
 * Define replacer for inode_operations structure.
 */

static operation_replacer inode_operations_replacer;
static int inode_operations_replacer_init(size_t elem_num)
{
	inode_operations_replacer = operation_replacer_create(elem_num,
		sizeof(struct inode_operations));
    if(inode_operations_replacer == NULL)
    {
        pr_err("inode_operations_replacer_init: Cannot create replacer for inode operations.");
        return -ENOMEM;
    }
    return 0;
}
static void inode_operations_replacer_destroy(void)
{
	operation_replacer_destroy(inode_operations_replacer);
}
static void inode_operations_target_load_callback(struct module* m)
{
	operation_target_load_callback(inode_operations_replacer, m);
}
static void inode_operations_undeleted_key_callback(void* key)
{
	pr_info("Inode operations for inode %p hasn't restored normally.",
		container_of(key, struct inode, i_op));
}
static void inode_operations_target_unload_callback(struct module* m)
{
	operation_target_unload_callback(inode_operations_replacer, m,
		inode_operations_undeleted_key_callback);
}
static int inode_operations_replace(struct inode* inode)
{
	return operation_replace(inode_operations_replacer, &inode->i_op);
}
static int inode_operations_restore(struct inode* inode)
{
	return operation_restore(inode_operations_replacer, &inode->i_op);
}
// Currently not used
/*static int inode_operations_replacement_clean(struct inode* inode)
{
	return operation_replacement_clean(inode_operations_replacer,
		&inode->i_op);
}*/
static int inode_operations_replacement_update(struct inode* inode)
{
	return operation_replacement_update(inode_operations_replacer,
		&inode->i_op);
}

int inode_operations_payload_register(struct operation_payload* payload)
{
	return operation_payload_register(inode_operations_replacer,
        payload);
}
int inode_operations_payload_unregister(struct operation_payload* payload)
{
	return operation_payload_unregister(inode_operations_replacer,
        payload);
}

void* inode_operations_get_orig_f(int op_offset, struct inode* inode)
{
    return operation_get_orig(inode_operations_replacer,
        op_offset, &inode->i_op);
}

int inode_operations_payload_register_special(
    struct operation_payload* payload)
{
	return operation_payload_register_special(inode_operations_replacer,
		payload);
}
int inode_operations_payload_unregister_special(
	struct operation_payload* payload)
{
	return operation_payload_unregister_special(inode_operations_replacer,
		payload);
}

void* inode_operations_get_orig_special_f(int op_offset,
    struct inode* inode, struct operation_payload* payload)
{
    return operation_get_orig_special(inode_operations_replacer,
        op_offset, &inode->i_op, payload);
}

/*
 * Define replacer for dentry_operations structure.
 */

static operation_replacer dentry_operations_replacer;
static int dentry_operations_replacer_init(size_t elem_num)
{
	dentry_operations_replacer = operation_replacer_create(elem_num,
		sizeof(struct dentry_operations));
    if(dentry_operations_replacer == NULL)
    {
        pr_err("dentry_operations_replacer_init: Cannot create replacer for dentry operations.");
        return -ENOMEM;
    }
    return 0;
}
static void dentry_operations_replacer_destroy(void)
{
	operation_replacer_destroy(dentry_operations_replacer);
}
static void dentry_operations_target_load_callback(struct module* m)
{
	operation_target_load_callback(dentry_operations_replacer, m);
}
static void dentry_operations_undeleted_key_callback(void* key)
{
	pr_info("dentry operations for dentry %p hasn't restored normally.",
		container_of(key, struct dentry, d_op));
}
static void dentry_operations_target_unload_callback(struct module* m)
{
	operation_target_unload_callback(dentry_operations_replacer, m,
		dentry_operations_undeleted_key_callback);
}
static int dentry_operations_replace(struct dentry* dentry)
{
	return operation_replace(dentry_operations_replacer, &dentry->d_op);
}
static int dentry_operations_restore(struct dentry* dentry)
{
	return operation_restore(dentry_operations_replacer, &dentry->d_op);
}
// Currently not used
/*static int dentry_operations_replacement_clean(struct dentry* dentry)
{
	return operation_replacement_clean(dentry_operations_replacer,
		&dentry->d_op);
}*/
static int dentry_operations_replacement_update(struct dentry* dentry)
{
	return operation_replacement_update(dentry_operations_replacer,
		&dentry->d_op);
}

int dentry_operations_payload_register(struct operation_payload* payload)
{
	return operation_payload_register(dentry_operations_replacer,
        payload);
}
int dentry_operations_payload_unregister(struct operation_payload* payload)
{
	return operation_payload_unregister(dentry_operations_replacer,
        payload);
}

void* dentry_operations_get_orig_f(int op_offset, struct dentry* dentry)
{
    return operation_get_orig(dentry_operations_replacer,
        op_offset, &dentry->d_op);
}

int dentry_operations_payload_register_special(
    struct operation_payload* payload)
{
	return operation_payload_register_special(dentry_operations_replacer,
		payload);
}
int dentry_operations_payload_unregister_special(
	struct operation_payload* payload)
{
	return operation_payload_unregister_special(dentry_operations_replacer,
		payload);
}

void* dentry_operations_get_orig_special_f(int op_offset,
    struct dentry* dentry, struct operation_payload* payload)
{
    return operation_get_orig_special(dentry_operations_replacer,
        op_offset, &dentry->d_op, payload);
}

/*
 * Define replacer for super_operations structure.
 */

static operation_replacer super_operations_replacer;
static int super_operations_replacer_init(size_t elem_num)
{
	super_operations_replacer = operation_replacer_create(elem_num,
		sizeof(struct super_operations));
    if(super_operations_replacer == NULL)
    {
        pr_err("super_operations_replacer_init: Cannot create replacer for super block operations.");
        return -ENOMEM;
    }
    return 0;
}
static void super_operations_replacer_destroy(void)
{
	operation_replacer_destroy(super_operations_replacer);
}
static void super_operations_target_load_callback(struct module* m)
{
	operation_target_load_callback(super_operations_replacer, m);
}
static void super_operations_undeleted_key_callback(void* key)
{
	pr_info("super_operations for super block %p hasn't restored normally.",
		container_of(key, struct super_block, s_op));
}
static void super_operations_target_unload_callback(struct module* m)
{
	operation_target_unload_callback(super_operations_replacer, m,
		super_operations_undeleted_key_callback);
}
static int super_operations_replace(struct super_block* super_block)
{
	return operation_replace(super_operations_replacer, &super_block->s_op);
}
static int super_operations_restore(struct super_block* super_block)
{
	return operation_restore(super_operations_replacer, &super_block->s_op);
}
static int super_operations_replacement_clean(struct super_block* super_block)
{
	return operation_replacement_clean(super_operations_replacer,
		&super_block->s_op);
}
static int super_operations_replacement_update(struct super_block* super_block)
{
	return operation_replacement_update(super_operations_replacer,
		&super_block->s_op);
}

int super_operations_payload_register(struct operation_payload* payload)
{
	return operation_payload_register(super_operations_replacer,
        payload);
}
int super_operations_payload_unregister(struct operation_payload* payload)
{
	return operation_payload_unregister(super_operations_replacer,
        payload);
}

void* super_operations_get_orig_f(int op_offset, struct super_block* super_block)
{
    return operation_get_orig(super_operations_replacer,
        op_offset, &super_block->s_op);
}

int super_operations_payload_register_special(
    struct operation_payload* payload)
{
	return operation_payload_register_special(super_operations_replacer,
		payload);
}
int super_operations_payload_unregister_special(
	struct operation_payload* payload)
{
	return operation_payload_unregister_special(super_operations_replacer,
		payload);
}

void* super_operations_get_orig_special_f(int op_offset,
    struct super_block* super_block, struct operation_payload* payload)
{
    return operation_get_orig_special(super_operations_replacer,
        op_offset, &super_block->s_op, payload);
}

/*
 * Define replacer for file_system_type structure.
 */

static operation_replacer file_system_type_replacer;
static int file_system_type_replacer_init(size_t elem_num)
{
	file_system_type_replacer = operation_replacer_create_at_place(elem_num,
		sizeof(struct file_system_type));
    if(file_system_type_replacer == NULL)
    {
        pr_err("file_system_type_replacer_init: Cannot create replacer for fs_type operations.");
        return -ENOMEM;
    }
    return 0;
}
static void file_system_type_replacer_destroy(void)
{
	operation_replacer_destroy(file_system_type_replacer);
}
static void file_system_type_target_load_callback(struct module* m)
{
	operation_target_load_callback(file_system_type_replacer, m);
}
static void file_system_type_undeleted_key_callback(void* key)
{
	pr_info("fs_type operations for type %p hasn't restored normally.",
		key);
}
static void file_system_type_target_unload_callback(struct module* m)
{
	operation_target_unload_callback(file_system_type_replacer, m,
		file_system_type_undeleted_key_callback);
}
static int file_system_type_replace(struct file_system_type* fs_type)
{
	return operation_replace(file_system_type_replacer, fs_type);
}
static int file_system_type_restore(struct file_system_type* fs_type)
{
	return operation_restore(file_system_type_replacer, fs_type);
}
// Currently not used
/*static int file_system_type_replacement_clean(struct file_system_type* fs_type)
{
	return operation_replacement_clean(file_system_type_replacer,
		fs_type);
}*/
// Currently not used
/*static int file_system_type_replacement_update(struct file_system_type* fs_type)
{
	return operation_replacement_update(file_system_type_replacer,
		fs_type);
}*/

int file_system_type_payload_register(struct operation_payload* payload)
{
	return operation_payload_register(file_system_type_replacer,
        payload);
}
int file_system_type_payload_unregister(struct operation_payload* payload)
{
	return operation_payload_unregister(file_system_type_replacer,
        payload);
}

void* file_system_type_get_orig_f(int op_offset, struct file_system_type* fs_type)
{
    return operation_get_orig(file_system_type_replacer,
        op_offset, fs_type);
}

int file_system_type_payload_register_special(
    struct operation_payload* payload)
{
	return operation_payload_register_special(file_system_type_replacer,
		payload);
}
int file_system_type_payload_unregister_special(
	struct operation_payload* payload)
{
	return operation_payload_unregister_special(file_system_type_replacer,
		payload);
}

void* file_system_type_get_orig_special_f(int op_offset,
    struct file_system_type* fs_type, struct operation_payload* payload)
{
    return operation_get_orig_special(file_system_type_replacer,
        op_offset, fs_type, payload);
}

/*
 * Define internal replacer for file_operations structure of inode(!).
 * 
 * Aside from standard operation replacer's functionality, this replacer
 * also save initial file operations of inode for copy it to file
 * at open() call.
 * 
 * Because of needs to save initial file operations, there is some
 * problems in implementing 'replacement_update' method.
 * From another side, this is very static replacer:
 * -it is 'bridge' between predefined replacers -
 *  inode_operations and file_operations
 * -it implements only one replacement operation.
 * -there is no chaining of operations.
 * 
 * So, it may be implemented without basic operations replacer's interfaces,
 * but by hand.
 */

// Forward declaration of replacement operation.
static int ifops_open__repl_fops(struct inode* inode, struct file* filp);

data_map_t inode_file_operations_map;

struct inode_file_operations_data
{
    const struct file_operations* ops_orig;
    struct file_operations ops_repl;
};
 
static int inode_file_operations_replacer_init(size_t elem_num)
{
    inode_file_operations_map = data_map_create(elem_num);
    if(inode_file_operations_map == NULL)
    {
        pr_err("Cannot create map for file operations of the inode.");
        return -ENOMEM;
    }
    
    return 0;
}
static void inode_file_operations_replacer_destroy(void)
{
	data_map_destroy(inode_file_operations_map);
}
static void inode_file_operations_target_load_callback(struct module* m)
{
    //only bridge to underlay layer
    file_operations_target_load_callback(m);
}
static void inode_file_operations_free_data(void* data,
    void* key, void* user_data)
{
	(void)user_data;
    kfree(data);
    pr_info("File operations for inode %p hasn't restored normally.",
		key);
}
static void inode_file_operations_target_unload_callback(struct module* m)
{
    data_map_delete_all(inode_file_operations_map,
        inode_file_operations_free_data, NULL);
    file_operations_target_unload_callback(m);
}
static int inode_file_operations_replace(struct inode* inode)
{
	int result;
    struct inode_file_operations_data* data =
        kmalloc(sizeof(*data), GFP_KERNEL);
    if(data == NULL)
    {
        pr_err("inode_file_operations_replace: Cannot allocate data for replace inode's file operations.");
        return -ENOMEM;
    }
    data->ops_orig = inode->i_fop;
    memcpy(&data->ops_repl, data->ops_orig, sizeof(data->ops_repl));
    data->ops_repl.open = ifops_open__repl_fops;
    
    result = data_map_add(inode_file_operations_map, inode, data);
    if(result)
    {
        kfree(data);
        return result;
    }
    inode->i_fop = &data->ops_repl;
    
    return 0;
}
static int inode_file_operations_restore(struct inode* inode)
{
	struct inode_file_operations_data* data =
        data_map_get(inode_file_operations_map, inode);
    
    if(IS_ERR(data))
    {
        pr_info("inode_file_operations_restore: There is no replacemt file operations for inode %p",
            inode);
        return -EINVAL;
    }
    BUG_ON(data == NULL);

    if(inode->i_fop != &data->ops_repl)
    {
        pr_info("File operations for inode %p was changed outside of replacer",
            inode);
    }
    else
    {
        inode->i_fop = data->ops_orig;
    }
    data_map_delete(inode_file_operations_map, inode);
    kfree(data);
    return 0;
}
// Currently not used
/*static int inode_file_operations_replacement_clean(struct inode* inode)
{
	data_map_delete(inode_file_operations_map, inode);
    return operation_replacement_clean(inode_file_operations_replacer,
		&inode->i_fop);
}*/

static int inode_file_operations_replacement_update(struct inode* inode)
{
	struct inode_file_operations_data* data =
        data_map_get(inode_file_operations_map, inode);
    
    if(IS_ERR(data))
    {
        return inode_file_operations_replace(inode);
    }
    BUG_ON(data == NULL);
    if(inode->i_fop == &data->ops_repl)
    {
        //nothing was changed
        return 0;
    }
    else if(inode->i_fop == data->ops_orig)
    {
        //operations was restored outside of replacer
        //simple replace pointer again
        inode->i_fop = &data->ops_repl;
        return 0;
    }
    //operations was really changed. Strange but possible
    data->ops_orig = inode->i_fop;
    memcpy(&data->ops_repl, data->ops_orig, sizeof(data->ops_repl));
    data->ops_repl.open = ifops_open__repl_fops;
    inode->i_fop = &data->ops_repl;
        
    return 0;
}

// Function special for replacer of file operations in inode.
static inline const struct file_operations*
inode_file_operations_get_orig_operations(struct inode* inode)
{
	struct inode_file_operations_data* data =
        data_map_get(inode_file_operations_map, inode);
    BUG_ON(IS_ERR(data));
    BUG_ON(data == NULL);
    return data->ops_orig;
}

//Replacer for 'fill_super' callback
static callback_replacer fill_super_replacer;
typedef int (*fill_super_t)(struct super_block *sb, void* data, int silent);
static int fill_super_replacer_init(size_t elem_num)
{
    fill_super_replacer = callback_replacer_create(elem_num);
    if(fill_super_replacer == NULL)
    {
        pr_err("fill_super_replacer_init: Cannot create replacer for 'fill_super' callback.");
        return -ENOMEM;
    }
    return 0;
}
static void fill_super_replacer_destroy(void)
{
    callback_replacer_destroy(fill_super_replacer);
}
static void fill_super_target_load_callback(struct module* m)
{
    callback_target_load_callback(fill_super_replacer, m);
}
static void fill_super_undeleted_key_callback(void* key)
{
    pr_info("'fill_super' callback for data %p wasn't cleaned normally.", key);
}
static void fill_super_target_unload_callback(struct module* m)
{
    callback_target_unload_callback(fill_super_replacer, m,
        fill_super_undeleted_key_callback);
}
static int fill_super_payload_register(struct callback_payload* payload)
{
    return callback_payload_register(fill_super_replacer, payload);
}
static int fill_super_payload_unregister(struct callback_payload* payload)
{
    return callback_payload_unregister(fill_super_replacer, payload);
}
static fill_super_t
fill_super_replace(fill_super_t callback, void* data)
{
    return callback_replace(fill_super_replacer, callback, data);
}
static int fill_super_replacement_clean(void* data)
{
    return callback_replacement_clean(fill_super_replacer, data);
}
static fill_super_t
fill_super_get_orig(void* data)
{
    return callback_get_orig(fill_super_replacer, data);
}                                                                       \

//Interconnections between operations of different types

//**********Unify operations for newly created inode******************
/*
 * Replace inode operations and file operations for inode.
 */

static int inode_operations_all_replace(struct inode* inode)
{
    int result = inode_operations_replace(inode);
    if(result < 0) return result;

    result = inode_file_operations_replace(inode);
    if(result < 0)
    {
        inode_operations_restore(inode);
    }
    return result;
}

static int inode_operations_all_restore(struct inode* inode)
{
    int result1, result2;
    result1 = inode_file_operations_restore(inode);
    result2 = inode_operations_restore(inode);

    return result1 ? result1 : result2;
}

static int inode_operations_all_replacement_update(struct inode* inode)
{
    int result1, result2;
    result1 = inode_operations_replacement_update(inode);
    result2 = inode_file_operations_replacement_update(inode);

    return result1 ? result1 : result2;
}

static void inode_operations_all_target_load_callback(struct module* m)
{
    inode_operations_target_load_callback(m);
    inode_file_operations_target_load_callback(m);
}

static void inode_operations_all_target_unload_callback(struct module* m)
{
    inode_file_operations_target_unload_callback(m);
    inode_operations_target_unload_callback(m);
}

//****Unify operations for newly created dentry with inode**************
/*
 * Replacer dentry operations for dentry and all operations for 
 * its inode.
 */

static int dentry_operations_all_replace(struct dentry* dentry)
{
    int result = dentry_operations_replace(dentry);
    if(result < 0) return result;

    if(dentry->d_inode)
    {
        result = inode_operations_all_replace(dentry->d_inode);
        if(result < 0)
        {
            dentry_operations_restore(dentry);
        }
    }
    return result;
}

static int dentry_operations_all_replacement_update(struct dentry* dentry)
{
    int result = dentry_operations_replacement_update(dentry);
    if(result < 0) return result;

    if(dentry->d_inode)
    {
        int result1 = inode_operations_all_replacement_update(dentry->d_inode);
        if(result1) return result1;
    }
    return result;
}


static void dentry_operations_all_target_load_callback(struct module* m)
{
    dentry_operations_target_load_callback(m);
    inode_operations_all_target_load_callback(m);
}

static void dentry_operations_all_target_unload_callback(struct module* m)
{
    inode_operations_all_target_unload_callback(m);
    dentry_operations_target_unload_callback(m);
}

//*Unify operations for newly created super_block with root dentry and inode*
/*
 * Replace super operation for super block and all operations for
 * its root dentry.
 */
static int super_operations_all_replace(struct super_block* super)
{
    int result = super_operations_replace(super);
    if(result < 0) return result;

    if(super->s_root)
    {
        if(super->s_root->d_inode)
        {
            result = dentry_operations_all_replace(super->s_root);
        }
        else
        {
            pr_info("Root inode of super block %p is NULL.", super);
            result = dentry_operations_replace(super->s_root);
            
        }
        if(result < 0)
        {
            super_operations_restore(super);
        }
    }
    else
    {
        pr_info("Root dentry of super block %p is NULL.", super);
    }
    return result;
}
static int super_operations_all_replacement_update(struct super_block* super)
{
    int result = super_operations_replacement_update(super);
    if(result < 0) return result;

    if(super->s_root)
    {
        int result1;
        if(super->s_root->d_inode)
        {
            result1 = dentry_operations_all_replacement_update(super->s_root);
        }
        else
        {
            pr_info("Root inode of super block %p is NULL.", super);
            result1 = dentry_operations_replacement_update(super->s_root);
            
        }
        if(result1) return result1;
    }
    else
    {
        pr_info("Root dentry of super block %p is NULL.", super);
    }
    return result;
}

static void super_operations_all_target_load_callback(struct module* m)
{
    super_operations_target_load_callback(m);
    dentry_operations_all_target_load_callback(m);
}

static void super_operations_all_target_unload_callback(struct module* m)
{
    dentry_operations_all_target_unload_callback(m);
    super_operations_target_unload_callback(m);
}


//****************fill_super->super_operations_all********************
/*
 * Intercept moment when super block is filled and replace
 * all needed operations for it.
 */

static int fill_super__repl_sopsa(struct super_block *sb, void* data, int silent)
{
    int result;
    typeof(fill_super__repl_sopsa)* fill_super__orig =
        fill_super_get_orig(data);
    
    BUG_ON(fill_super__orig == NULL);

    result = fill_super__orig(sb, data, silent);
    
    super_operations_all_replace(sb);
    
    return result;
}

struct callback_payload fill_super_sopsa_payload =
{
    .m = THIS_MODULE,
    .callback = fill_super__repl_sopsa,
    .target_load_callback = super_operations_all_target_load_callback,
    .target_unload_callback = super_operations_all_target_unload_callback,
};

//************file_system_type->super_operations_all*****************
/*
 * Intercept moment when super block is filled and replacer
 * all needed operations for it.
 * 
 * Intercept moment when super block is destroyed and restore
 * super operations for it.
 */

//Forward declarations of replacement operations
static int fst_get_sb__repl_sopsa(struct file_system_type *fst,
    int flags, const char *dev_name, void *rawData, struct vfsmount *mnt);

static void fst_kill_sb__repl_sopsa(struct super_block *sb);

static struct file_system_type fst_repl_sopsa =
{
    .get_sb = fst_get_sb__repl_sopsa,
    .kill_sb = fst_kill_sb__repl_sopsa,
};

static struct file_system_type fst_mask_sopsa = 
{
    .get_sb = REPLACEMENT_MASK,
    .kill_sb = REPLACEMENT_MASK,
};

struct operation_payload fst_sopsa_payload =
{
    .m = THIS_MODULE,
    .repl = &fst_repl_sopsa,
    .mask = &fst_mask_sopsa,
    .target_load_callback = super_operations_all_target_load_callback,
    .target_unload_callback = super_operations_all_target_unload_callback,
};

#define fst_sopsa_get_ops_orig(op, fs_type) \
    file_system_type_get_orig_special(op, fs_type, &fst_sopsa_payload)

int fst_get_sb__repl_sopsa(struct file_system_type *fst,
    int flags, const char *dev_name, void *rawData, struct vfsmount *mnt)
{
    int result = fst_sopsa_get_ops_orig(get_sb, fst)
        (fst, flags, dev_name, rawData, mnt);
    
    if(result) return result;//error
    // Not simple operation_replace() because we may intercept fill_super callback
    // for some top-level functions.
    super_operations_all_replacement_update(mnt->mnt_sb);
    return 0;
}

void fst_kill_sb__repl_sopsa(struct super_block *sb)
{
    fst_sopsa_get_ops_orig(kill_sb, sb->s_type)(sb);
    //super block may be freed now, so do not restore its operations
    super_operations_replacement_clean(sb);
}

//*********super_block_operations->inode_operations_all******************
//Forward declarations of replacement operations

/*
 * Intercept moment when inode is destroyed and restore operations for it.
 *
 * Note, that operations for root inode was replaced in the
 * file_system_type::get_sb() replacement.
 * 
 * Operations for other inodes will be replaced in replacements for root inode(!!).
 */

static void sops_destroy_inode__repl_iopsa(struct inode* inode);
//Because we intercept destroy_inode, we should also intercept alloc_inode.
static struct inode* sops_alloc_inode__repl_iopsa(struct super_block* super);

static struct super_operations sops_repl_iopsa =
{
    .alloc_inode = sops_alloc_inode__repl_iopsa,
    .destroy_inode = sops_destroy_inode__repl_iopsa,
};

static struct super_operations sops_mask_iopsa =
{
    .alloc_inode = REPLACEMENT_MASK,
    .destroy_inode = REPLACEMENT_MASK,
};

static struct operation_payload sops_iopsa_payload=
{
    .m = THIS_MODULE,
    .repl = &sops_repl_iopsa,
    .mask = &sops_mask_iopsa,
    .target_load_callback = inode_operations_all_target_load_callback,
    .target_unload_callback = inode_operations_all_target_unload_callback,
};

#define sops_iopsa_get_ops_orig(op, super_block) \
    super_operations_get_orig_special(op, super_block, &sops_iopsa_payload)

void sops_destroy_inode__repl_iopsa(struct inode* inode)
{
    typeof(sops_destroy_inode__repl_iopsa)* op_orig =
        sops_iopsa_get_ops_orig(destroy_inode, inode->i_sb);
    inode_operations_all_restore(inode);
    
    if(op_orig)
        op_orig(inode);
    else
        super_operations_destroy_inode__default(inode);
}

struct inode* sops_alloc_inode__repl_iopsa(struct super_block* super)
{
    struct inode* inode;
    typeof(sops_alloc_inode__repl_iopsa)* op_orig =
        sops_iopsa_get_ops_orig(alloc_inode, super);
    inode = op_orig
        ? op_orig(super)
        : super_operations_alloc_inode__default(super);
    return inode;
}

//**********dentry_operations->dentry_operations_all******************
/*
 * Intercept moment when dentry is destroyed and restore operations for it.
 * 
 * Intercept some operations, which may set operations for dentry and it inode,
 * and update replacement for these operations.
 */

//Forward declarations of replacement operations
static void dops_d_release__repl_dopsa(struct dentry *dentry);
static int dops_d_revalidate__repl_dopsa(struct dentry *dentry,
	struct nameidata *nmi);

static struct dentry_operations dops_repl_dopsa =
{
    .d_release = dops_d_release__repl_dopsa,
    .d_revalidate = dops_d_revalidate__repl_dopsa,
};

static struct dentry_operations dops_mask_dopsa =
{
    .d_release = REPLACEMENT_MASK,
    .d_revalidate = REPLACEMENT_MASK,
};

static struct operation_payload dops_dopsa_payload = 
{
    .m = THIS_MODULE,
    .repl = &dops_repl_dopsa,
    .mask = &dops_mask_dopsa,

    .target_load_callback = dentry_operations_all_target_load_callback,
    .target_unload_callback = dentry_operations_all_target_unload_callback,
};

#define dops_dopsa_get_ops_orig(op, dentry) \
    dentry_operations_get_orig_special(op, dentry, &dops_dopsa_payload)

void dops_d_release__repl_dopsa(struct dentry *dentry)
{
    typeof(dops_d_release__repl_dopsa)* op_orig =
        dops_dopsa_get_ops_orig(d_release, dentry);

    dentry_operations_restore(dentry);//do not touch inode
    if(op_orig)
        op_orig(dentry);
    else
        dentry_operations_d_release__default(dentry);
}

int dops_d_revalidate__repl_dopsa(struct dentry *dentry,
	struct nameidata *nmi)
{
    int result;
    typeof(dops_d_revalidate__repl_dopsa)* op_orig =
        dops_dopsa_get_ops_orig(d_revalidate, dentry);
    
    result = op_orig
        ? op_orig(dentry, nmi)
        : dentry_operations_d_revalidate__default(dentry, nmi);
    if(result == 0) return 0;
    dentry_operations_all_replacement_update(dentry);
    
    return result;
}


//***************inode_operations->dentry_operations_all***************
/*
 * Intercept operations which create dentries with new inodes
 * and replace all needed operations.
 * 
 * Intercept some operations which may set operations for dentry and inode,
 * and update replacement for them.
 */

//Forward declaration of replacement operations
static struct dentry* iops_lookup__repl_dopsa(struct inode *inode,
	struct dentry *dentry, struct nameidata *nmi);
static int iops_create__repl_dopsa(struct inode *dir,
	struct dentry *dentry, int mode, struct nameidata *nd);
static int
iops_mkdir__repl_dopsa(struct inode *dir,
    struct dentry *dentry, int mode);
static int
iops_rename__repl_dopsa(struct inode *oldDir, struct dentry *oldDentry,
           struct inode *newDir, struct dentry *newDentry);
static int
iops_symlink__repl_dopsa(struct inode *dir,
            struct dentry *dentry, const char *symname);
//For updating replacement in case operations was changed inside operation
static int iops_getattr__repl_dopsa(struct vfsmount *mnt, 
	struct dentry *dentry, struct kstat *stat);
static int
iops_setattr__repl_dopsa(struct dentry *dentry, struct iattr *iattr);


static struct inode_operations iops_repl_dopsa =
{
    .lookup = iops_lookup__repl_dopsa,
    .create = iops_create__repl_dopsa,
    .mkdir = iops_mkdir__repl_dopsa,
    .rename = iops_rename__repl_dopsa,
    .symlink = iops_symlink__repl_dopsa,
    .getattr = iops_getattr__repl_dopsa,
    .setattr = iops_setattr__repl_dopsa,
};

static struct inode_operations iops_mask_dopsa =
{
    .lookup = REPLACEMENT_MASK,
    .create = REPLACEMENT_MASK,
    .mkdir = REPLACEMENT_MASK,
    .rename = REPLACEMENT_MASK,
    .symlink = REPLACEMENT_MASK,
    .getattr = REPLACEMENT_MASK,
    .setattr = REPLACEMENT_MASK,
};

static struct operation_payload iops_dopsa_payload =
{
    .m = THIS_MODULE,
    .repl = &iops_repl_dopsa,
    .mask = &iops_mask_dopsa,

    .target_load_callback = dentry_operations_all_target_load_callback,
    .target_unload_callback = dentry_operations_all_target_unload_callback,
};

#define iops_dopsa_get_ops_orig(op, inode) \
    inode_operations_get_orig_special(op, inode, &iops_dopsa_payload)

static struct dentry* iops_lookup__repl_dopsa(struct inode *inode,
	struct dentry *dentry, struct nameidata *nmi)
{
    struct dentry* dentry_result;
    typeof(iops_lookup__repl_dopsa)* op_orig =
        iops_dopsa_get_ops_orig(lookup, inode);
        
    dentry_result = op_orig
        ? op_orig(inode, dentry, nmi)
        : inode_operations_lookup__default(inode, dentry, nmi);
    
    if(dentry_result == NULL)
    {
        //pr_info("inode_operations::lookup() result is NULL.");
    }
    else if(IS_ERR(dentry_result))
    {
        //pr_info("inode_operations::lookup() result is error %ld.",
        //    PTR_ERR(dentry_result));
    }
    else
    {
        pr_info("inode_operations::lookup(): Replace operations for new dentry %p (caller inode is %p).",
            dentry_result, inode);
        dentry_operations_all_replace(dentry_result);
    }
    
    dentry_operations_all_replacement_update(dentry);

    return dentry_result;
}

static int iops_create__repl_dopsa(struct inode *dir,
	struct dentry *dentry, int mode, struct nameidata *nd)
{
    int result;
    typeof(iops_create__repl_dopsa)* op_orig =
        iops_dopsa_get_ops_orig(create, dir);
    
    result = op_orig
        ? op_orig(dir, dentry, mode, nd)
        : inode_operations_create__default(dir, dentry, mode, nd);
    
    if(result) return result;
    
    dentry_operations_all_replacement_update(dentry);
    
    return result;
}

static int
iops_mkdir__repl_dopsa(struct inode *dir,
    struct dentry *dentry, int mode)
{
    int result;
    typeof(iops_mkdir__repl_dopsa)* op_orig =
        iops_dopsa_get_ops_orig(mkdir, dir);
    
    result = op_orig
        ? op_orig(dir, dentry, mode)
        : inode_operations_mkdir__default(dir, dentry, mode);
    if(result) return result;
    
    dentry_operations_all_replacement_update(dentry);
    
    return 0;
}

static int
iops_rename__repl_dopsa(struct inode *oldDir, struct dentry *oldDentry,
           struct inode *newDir, struct dentry *newDentry)
{
    int result;
    typeof(iops_rename__repl_dopsa)* op_orig =
        iops_dopsa_get_ops_orig(rename, oldDir);
    
    result = op_orig
        ? op_orig(oldDir, oldDentry, newDir, newDentry)
        : inode_operations_rename__default(oldDir, oldDentry, newDir, newDentry);
    if(result) return result;
    
    dentry_operations_all_replacement_update(newDentry);
    
    return 0;
}

static int
iops_symlink__repl_dopsa(struct inode *dir,
            struct dentry *dentry, const char *symname)
{
    int result;
    typeof(iops_symlink__repl_dopsa)* op_orig =
        iops_dopsa_get_ops_orig(symlink, dir);
    
    result = op_orig
        ? op_orig(dir, dentry, symname)
        : inode_operations_symlink__default(dir, dentry, symname);
    if(result) return result;
    
    dentry_operations_all_replacement_update(dentry);
    return 0;
}
//For updating replacement in case operations was changed inside operation
static int iops_getattr__repl_dopsa(struct vfsmount *mnt, 
	struct dentry *dentry, struct kstat *stat)
{
    int result;
    typeof(iops_getattr__repl_dopsa)* op_orig =
        iops_dopsa_get_ops_orig(getattr, dentry->d_inode);

    result = op_orig
        ? op_orig(mnt, dentry, stat)
        : inode_operations_getattr__default(mnt, dentry, stat);
    
    dentry_operations_all_replacement_update(dentry);
    
    return result;
}
static int
iops_setattr__repl_dopsa(struct dentry *dentry, struct iattr *iattr)
{
    int result;
    typeof(iops_setattr__repl_dopsa)* op_orig =
        iops_dopsa_get_ops_orig(setattr, dentry->d_inode);

    result = op_orig
        ? op_orig(dentry, iattr)
        : inode_operations_setattr__default(dentry, iattr);
    
    dentry_operations_all_replacement_update(dentry);
    
    return result;
}

//*************inode_file_operations->file_operations******************

/*
 * Intercept moment when file is opened and replace operations for it.
 * 
 * Note: inode_file_operations' replacer will be implemented
 * without standard operation_replacer's interface.
 * So only implement replacement operation here.
 */
static int ifops_open__repl_fops(struct inode* inode, struct file* filp)
{
    int result;

    /*
     * At this stage VFS already set file operations for the file
     * equal to that in inode, that is, which already replaced.
     * 
     * Firstly we set unmodified operations for file.
     * 
     * This is reason why we do not use standard operation_replacer's
     * interface - we need structure with all original operations,
     * not for one of them.
     */
    
    const struct file_operations* ops_orig =
        inode_file_operations_get_orig_operations(inode);

    filp->f_op = ops_orig;
    
    // Then replace operations according to registered payloads.
    
    file_operations_replace(filp);
    
    /*
     * Next, chain open() call.
     * 
     * But instead of using standard replacer's mechanism,
     * call open() using f_op field of the file.
     * This is another reason why we do not use standard
     * operation_replacer's interface.
     */
    
    result = filp->f_op->open
        ? filp->f_op->open(inode, filp)
        : file_operations_open__default(inode, filp);
    
    if(result)
    {
        file_operations_restore(filp);
        return result;
    }
    
    /*
     * Open function may change file operations.
     * So, update replacement.
     */
    
    file_operations_replacement_update(filp);
    
    return 0;
}

//*************file_operations->inode_operations_all*************************
/*
 * Intercept some moments when operations for inode,
 * corresponded to the file, may be set, and update replacement for them.
 */

//Forward declaration of replacement operations
static int fops_release__repl_iopsa(struct inode* inode, struct file* filp);

static struct file_operations fops_repl_iopsa =
{
    .release = fops_release__repl_iopsa,
};

static struct file_operations fops_mask_iopsa =
{
    .release = REPLACEMENT_MASK,
};

static struct operation_payload fops_iopsa_payload =
{
    .m = THIS_MODULE,
    .repl = &fops_repl_iopsa,
    .mask = &fops_mask_iopsa,

    .target_load_callback = inode_operations_all_target_load_callback,
    .target_unload_callback = inode_operations_all_target_unload_callback,
};

#define fops_iopsa_get_ops_orig(op, filp) \
    file_operations_get_orig_special(op, filp, &fops_iopsa_payload)

static int fops_release__repl_iopsa(struct inode* inode, struct file* filp)
{
    int result;
    typeof(fops_release__repl_iopsa)* op_orig =
        fops_iopsa_get_ops_orig(release, filp);
    
    result = op_orig
        ? op_orig(inode, filp)
        : file_operations_release__default(inode, filp);
    
    if(result) return result;
    
    inode_operations_all_replacement_update(inode);

    return 0;
}

//*************file_operations->file_operations*************************
/*
 * Intercept moment when file is closed and restore operations for it.
 */
//Forward declaration of replacement operations
static int fops_release__repl_fops(struct inode* inode, struct file* filp);

static struct file_operations fops_repl_fops =
{
    .release = fops_release__repl_fops,
};

static struct file_operations fops_mask_fops =
{
    .release = REPLACEMENT_MASK,
};

static struct operation_payload fops_fops_payload =
{
    .m = THIS_MODULE,
    .repl = &fops_repl_fops,
    .mask = &fops_mask_fops,

    .target_load_callback = file_operations_target_load_callback,
    .target_unload_callback = file_operations_target_unload_callback,
};

#define fops_fops_get_ops_orig(op, filp) \
    file_operations_get_orig_special(op, filp, &fops_fops_payload)

static int fops_release__repl_fops(struct inode* inode, struct file* filp)
{
    int result;
    typeof(fops_release__repl_fops)* op_orig =
        fops_fops_get_ops_orig(release, filp);
    
    result = op_orig
        ? op_orig(inode, filp)
        : file_operations_release__default(inode, filp);
    
    if(result) return result;
    
    file_operations_restore(filp);

    return 0;
}

//************KEDR replacements->file_system_type******************
/*
 * Intercept moment when file_system_type is registered
 * and replace operations for it.
 * 
 * Intercept moment when file_system_type is deregistered
 * and restore operations for it.
 */

static int register_filesystem__repl_fst(struct file_system_type *fst)
{
    int result;
    file_system_type_replace(fst);
    result = register_filesystem(fst);
    if(result)
    {
        file_system_type_restore(fst);
    }
    return result;
}

static int unregister_filesystem__repl_fst(struct file_system_type *fst)
{
    int result = unregister_filesystem(fst);
    if(result) return result;
    file_system_type_restore(fst);
    return 0;
}

static void* orig_addrs_fst[] = {
    (void*)&register_filesystem,
    (void*)&unregister_filesystem,
};

static void* repl_addrs_fst[] = {
    (void*)&register_filesystem__repl_fst,
    (void*)&unregister_filesystem__repl_fst,
};

static struct kedr_payload kedr_fst_payload = {
    .mod                    = THIS_MODULE,
    .repl_table.orig_addrs  = &orig_addrs_fst[0],
    .repl_table.repl_addrs  = &repl_addrs_fst[0],
    .repl_table.num_addrs   = ARRAY_SIZE(orig_addrs_fst),
    .target_load_callback   = file_system_type_target_load_callback,
    .target_unload_callback = file_system_type_target_unload_callback,
};


//************KEDR replacements->fill_super callback******************
/*
 * Intercept some functions which may be used by the implementation
 * of file_system_type::get_sb operation for create super block for
 * fill_super callback.
 */

static int get_sb_bdev__repl_fill_super(struct file_system_type *fs_type,
    int flags, const char *dev_name, void *data,
    int (*fill_super)(struct super_block *, void *, int),
    struct vfsmount *mnt)
{
    int result;

    fill_super_t fill_super_repl = fill_super_replace(fill_super, data);
    if(IS_ERR(fill_super_repl))
    {
        pr_err("Fail to perform fill_super callback replacement.");
        return get_sb_bdev(fs_type, flags, dev_name, data, fill_super, mnt);
    }
    result = get_sb_bdev(fs_type, flags, dev_name, data, fill_super_repl, mnt);
    fill_super_replacement_clean(data);
    
    return result;
}

static int get_sb_nodev__repl_fill_super(struct file_system_type *fs_type,
    int flags, void *data,
    int (*fill_super)(struct super_block *, void *, int),
    struct vfsmount *mnt)
{
    int result;

    fill_super_t fill_super_repl = fill_super_replace(fill_super, data);
    if(IS_ERR(fill_super_repl))
    {
        pr_err("Fail to perform fill_super callback replacement.");
        return get_sb_nodev(fs_type, flags, data, fill_super, mnt);
    }
    result = get_sb_nodev(fs_type, flags, data, fill_super_repl, mnt);
    fill_super_replacement_clean(data);
    
    return result;
}

int get_sb_single__repl_fill_super(struct file_system_type *fs_type,
    int flags, void *data,
    int (*fill_super)(struct super_block *, void *, int),
    struct vfsmount *mnt)
{
    int result;

    fill_super_t fill_super_repl = fill_super_replace(fill_super, data);
    if(IS_ERR(fill_super_repl))
    {
        pr_err("Fail to perform fill_super callback replacement.");
        return get_sb_single(fs_type, flags, data, fill_super, mnt);
    }
    result = get_sb_single(fs_type, flags, data, fill_super_repl, mnt);
    fill_super_replacement_clean(data);
    
    return result;
}

static void* orig_addrs_fill_super[] = {
    (void*)&get_sb_single,
    (void*)&get_sb_nodev,
    (void*)&get_sb_bdev,
};

static void* repl_addrs_fill_super[] = {
    (void*)&get_sb_single__repl_fill_super,
    (void*)&get_sb_nodev__repl_fill_super,
    (void*)&get_sb_bdev__repl_fill_super,
};

static struct kedr_payload kedr_fill_super_payload = {
    .mod                    = THIS_MODULE,
    .repl_table.orig_addrs  = &orig_addrs_fill_super[0],
    .repl_table.repl_addrs  = &repl_addrs_fill_super[0],
    .repl_table.num_addrs   = ARRAY_SIZE(orig_addrs_fill_super),
    .target_load_callback   = fill_super_target_load_callback,
    .target_unload_callback = fill_super_target_unload_callback,
};

// Initialize all replacers and set its interconnections.
int fs_operation_replacement_init(void)
{
    int result;
    
    // Initialize all subsystems
    result = file_system_type_replacer_init(5);
    if(result) goto err_fst_init;
    result = super_operations_replacer_init(5);
    if(result) goto err_sops_init;
    result = inode_operations_replacer_init(1000);
    if(result) goto err_iops_init;
    result = dentry_operations_replacer_init(100);
    if(result) goto err_dops_init;
    result = inode_file_operations_replacer_init(10);
    if(result) goto err_inode_fops_init;
    result = file_operations_replacer_init(20);
    if(result) goto err_fops_init;
    
    result = fill_super_replacer_init(1);
    if(result) goto err_fill_super_init;

    // Set interconnections between subsystems
    result = kedr_payload_register(&kedr_fst_payload);
    if(result) goto err_kedr_fst_payload;
    result = kedr_payload_register(&kedr_fill_super_payload);
    if(result) goto err_kedr_fill_super_payload;
    result = file_system_type_payload_register_special(&fst_sopsa_payload);
    if(result) goto err_fst_sopsa_payload;
    result = fill_super_payload_register(&fill_super_sopsa_payload);
    if(result) goto err_fill_super_sopsa_payload;
    result = super_operations_payload_register_special(&sops_iopsa_payload);
    if(result) goto err_sops_iopsa_payload;
    result = dentry_operations_payload_register_special(&dops_dopsa_payload);
    if(result) goto err_dops_dopsa_payload;
    result = inode_operations_payload_register_special(&iops_dopsa_payload);
    if(result) goto err_iops_dopsa_payload;
    result = file_operations_payload_register_special(&fops_iopsa_payload);
    if(result) goto err_fops_iopsa_payload;
    result = file_operations_payload_register_special(&fops_fops_payload);
    if(result) goto err_fops_fops_payload;

    
    return 0;

//    file_operations_payload_unregister_special(&fops_fops_payload);
err_fops_fops_payload:
    file_operations_payload_unregister_special(&fops_iopsa_payload);
err_fops_iopsa_payload:
    inode_operations_payload_unregister_special(&iops_dopsa_payload);
err_iops_dopsa_payload:
    dentry_operations_payload_unregister_special(&dops_dopsa_payload);
err_dops_dopsa_payload:
    super_operations_payload_unregister_special(&sops_iopsa_payload);
err_sops_iopsa_payload:
    fill_super_payload_unregister(&fill_super_sopsa_payload);
err_fill_super_sopsa_payload:
    file_system_type_payload_unregister_special(&fst_sopsa_payload);
err_fst_sopsa_payload:
    kedr_payload_unregister(&kedr_fill_super_payload);
err_kedr_fill_super_payload:
    kedr_payload_unregister(&kedr_fst_payload);
err_kedr_fst_payload:
    fill_super_replacer_destroy();

err_fill_super_init:    
    file_operations_replacer_destroy();
err_fops_init:
    inode_file_operations_replacer_destroy();
err_inode_fops_init:
    dentry_operations_replacer_destroy();
err_dops_init:    
    inode_operations_replacer_destroy();
err_iops_init:
    super_operations_replacer_destroy();
err_sops_init:
    file_system_type_replacer_destroy();
err_fst_init:
    return result;
}

// Destroy all replacers.
void fs_operation_replacement_destroy(void)
{
    file_operations_payload_unregister_special(&fops_fops_payload);
    file_operations_payload_unregister_special(&fops_iopsa_payload);
    inode_operations_payload_unregister_special(&iops_dopsa_payload);
    dentry_operations_payload_unregister_special(&dops_dopsa_payload);
    super_operations_payload_unregister_special(&sops_iopsa_payload);
    fill_super_payload_unregister(&fill_super_sopsa_payload);
    file_system_type_payload_unregister_special(&fst_sopsa_payload);
    kedr_payload_unregister(&kedr_fst_payload);

    fill_super_replacer_destroy();
    file_operations_replacer_destroy();
    inode_file_operations_replacer_destroy();
    dentry_operations_replacer_destroy();
    inode_operations_replacer_destroy();
    super_operations_replacer_destroy();
    file_system_type_replacer_destroy();
}