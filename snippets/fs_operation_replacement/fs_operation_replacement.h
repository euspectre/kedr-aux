#ifndef FS_OPERATION_REPLACEMENT_H
#define FS_OPERATION_REPLACEMENT_H

#include "operation_replacer.h"
#include <linux/fs.h>

int fs_operation_replacement_init(void);
void fs_operation_replacement_destroy(void);


//Macros for implement replacer-specific macros
#define operation_replacer_get_orig_common(op, key, prefix, ops_type) \
	((typeof(((ops_type*)0)->op)) \
    prefix##_get_orig_f(offsetof(ops_type, op), key))

#define operation_replacer_get_orig_special_common(op, key, payload, prefix, ops_type) \
	((typeof(((ops_type*)0)->op)) \
    prefix##_get_orig_special_f(offsetof(ops_type, op), key, payload))


// Manage of replacements for file_system_type operations

/*
 * Register payload for replace operations of file_system_type.
 */
int
file_system_type_payload_register(struct operation_payload* payload);

/*
 * Deregister payload for replace operations of file_system_type.
 */
int
file_system_type_payload_unregister(struct operation_payload* payload);

/*
 * Return pointer to operation which is replaced by one at offset
 * 'op_offset' in the file_system_type structure.
 * 
 * Intended to use inside the replacement operation.
 */
void*
file_system_type_get_orig_f(int op_offset,
    struct file_system_type* fs_type);

/*
 * Same as file_system_type_get_orig_f() but use operation name instead 
 * of its offset in the file_system_type structure.
 * 
 * Result is automatically casted to the type of this operation.
 */
#define file_system_type_get_orig(op, fs_type) \
    operation_replacer_get_orig_common(op, fs_type, file_system_type, struct file_system_type)

/*
 * Register special payload for replace operations of file_system_type.
 */
int
file_system_type_payload_register_special(struct operation_payload* payload);

/*
 * Deregister special payload for replace operations of file_system_type.
 */
int
file_system_type_payload_unregister_special(struct operation_payload* payload);

/*
 * Return pointer to operation which is replaced by one at offset
 * 'op_offset' in the file_system_type structure.
 * 
 * Intended to use inside the replacement operation, which is registered
 * via special payload.
 */
void*
file_system_type_get_orig_special_f(int op_offset,
    struct file_system_type* fs_type,
    struct operation_payload* payload);

/*
 * Same as file_system_type_get_orig_special_f()
 * but use operation name instead of its offset
 * in the file_system_type structure.
 * 
 * Result is automatically casted to the type of this operation.
 */
#define file_system_type_get_orig_special(op, fs_type, payload) \
    operation_replacer_get_orig_special_common(op, fs_type, payload, file_system_type, struct file_system_type)


/*
 *  Manage of replacements for super_operations of super_block.
 * 
 * Meaning of these functions is similar to ones for 'file_system_type'.
 */
int
super_operations_payload_register(
    struct operation_payload* payload);
int
super_operations_payload_unregister(
    struct operation_payload* payload);

void*
super_operations_get_orig_f(int op_offset,
    struct super_block* super_block);

#define super_operations_get_orig(op, super_block) \
    operation_replacer_get_orig_common(op, super_block, super_operations, struct super_operations)


int
super_operations_payload_register_special(
    struct operation_payload* payload);
int
super_operations_payload_unregister_special(
    struct operation_payload* payload);

void*
super_operations_get_orig_special_f(int op_offset,
    struct super_block* super_block,
    struct operation_payload* payload);

#define super_operations_get_orig_special(op, super_block, payload) \
    operation_replacer_get_orig_special_common(op, super_block, payload, super_operations, struct super_operations)


/*
 * Manage of replacements for inode_operations of inode.
 * 
 * Meaning of these functions is similar to ones for 'file_system_type'.
 */

int
inode_operations_payload_register(
    struct operation_payload* payload);
int
inode_operations_payload_unregister(
    struct operation_payload* payload);

void* inode_operations_get_orig_f(int op_offset,
    struct inode* inode);

#define inode_operations_get_orig(op, inode) \
    operation_replacer_get_orig_common(op, inode, inode_operations, struct inode_operations)


int
inode_operations_payload_register_special(
    struct operation_payload* payload);
int
inode_operations_payload_unregister_special(
    struct operation_payload* payload);

void*
inode_operations_get_orig_special_f(int op_offset,
    struct inode* inode,
    struct operation_payload* payload);

#define inode_operations_get_orig_special(op, inode, payload) \
    operation_replacer_get_orig_special_common(op, inode, payload, inode_operations, struct inode_operations)

/*
 * Manage of replacement for dentry_operations of dentry.
 *
 * Meaning of these functions is similar to ones for 'file_system_type'.
 */
int
dentry_operations_payload_register(
    struct operation_payload* payload);
int
dentry_operations_payload_unregister(
    struct operation_payload* payload);

void*
dentry_operations_get_orig_f(int op_offset,
    struct dentry* dentry);
    
#define dentry_operations_get_orig(op, dentry) \
    operation_replacer_get_orig_common(op, dentry, dentry_operations, struct dentry_operations)


int
dentry_operations_payload_register_special(
    struct operation_payload* payload);
int
dentry_operations_payload_unregister_special(
    struct operation_payload* payload);

void*
dentry_operations_get_orig_special_f(int op_offset,
    struct dentry* dentry,
    struct operation_payload* payload);

#define dentry_operations_get_orig_special(op, dentry, payload) \
    operation_replacer_get_orig_special_common(op, dentry, payload, dentry_operations, struct dentry_operations)

/*
 * Manage of replacements for file_operations of file.
 * 
 * Meaning of these functions is similar to ones for 'file_system_type'.
 */
int
file_operations_payload_register(
    struct operation_payload* payload);
int
file_operations_payload_unregister(
    struct operation_payload* payload);

void*
file_operations_get_orig_f(int op_offset,
    struct file* filp);

#define file_operations_get_orig(op, filp) \
    operation_replacer_get_orig_common(op, filp, file_operations, struct file_operations)

int
file_operations_payload_register_special(
    struct operation_payload* payload);
int
file_operations_payload_unregister_special(
    struct operation_payload* payload);

void*
file_operations_get_orig_special_f(int op_offset,
    struct file* filp,
    struct operation_payload* payload);

#define file_operations_get_orig_special(op, filp, payload) \
    operation_replacer_get_orig_special_common(op, filp, payload, file_operations, struct file_operations)


/*
 * Default implementation of the operations.
 * 
 * The thing is that some operations may be not set(that is, set to NULL).
 * 
 * When encounter such operations, VFS treated them
 * in some predefined manner.
 * When one intercept operation, which originally was NULL, he shouldn't
 * call NULL operations as a original operation call, but should use
 * some code, which result is treated by VFS same as the absence
 * of the operation.
 * 
 * Below is code which result is similar to the corresponded NULL-operations.
 */

// Default implementation of file_system_type operations
static inline int
file_system_type_get_sb__default(struct file_system_type *fst,
    int flags, const char *dev_name, void *rawData, struct vfsmount *mnt)
{
    BUG();//NULL pointer as get_sb() operation is inacceptable.

    return -EPERM;
}

static inline void
file_system_type_kill_sb__default(struct super_block *sb)
{
    BUG();//NULL pointer as kill_sb() operation is inacceptable.
}

// Default implementation of super_operations

/* 
 * !!NOTE: alloc_inode() and destroy_inode() should be either
 * intercepted both or is not intercepted at all.
 */
static inline struct inode*
super_operations_alloc_inode__default(struct super_block* super)
{
    struct inode* inode = kmalloc(sizeof(*inode), GFP_KERNEL);
    //Usually this is done in the kmem_cache constructor for inode.
    if(inode) inode_init_once(inode);
    
    return inode;
}

static inline void super_operations_destroy_inode__default(struct inode* inode)
{
    kfree(inode);
}
// Default implementation of dentry_operations
static inline void
dentry_operations_d_release__default(struct dentry *dentry)
{
    //do nothing
}

static inline int
dentry_operations_d_revalidate__default(struct dentry *dentry,
	struct nameidata *nmi)
{
    return 0;//Means "need lookup".
}

// Default implementation of inode_operations
static inline struct dentry*
inode_operations_lookup__default(struct inode *inode,
	struct dentry *dentry, struct nameidata *nmi)
{
    return ERR_PTR(-ENOTDIR);
}

static inline int inode_operations_create__default(struct inode *dir,
	struct dentry *dentry, int mode, struct nameidata *nd)
{
    return -EACCES;
}

static inline int
inode_operations_mkdir__default(struct inode *dir,
    struct dentry *dentry, int mode)
{
    return -EPERM;
}

static inline int
inode_operations_rename__default(struct inode *oldDir, struct dentry *oldDentry,
           struct inode *newDir, struct dentry *newDentry)
{
    return -EPERM;
}

static inline int
inode_operations_symlink__default(struct inode *dir,
            struct dentry *dentry, const char *symname)
{
    return -EPERM;
}

static inline int
inode_operations_permission__default(struct inode *inode, int mask)
{
    //not implemented yet
    return 0;
}

static inline int inode_operations_getattr__default(struct vfsmount *mnt, 
	struct dentry *dentry, struct kstat *stat)
{
    generic_fillattr(dentry->d_inode, stat);
    return 0;
}

static inline int
inode_operations_setattr__default(struct dentry *dentry,
    struct iattr *iattr)
{
//#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36)
//    return simple_setattr(dentry, iattr);
//#else
// Not sure that this call is really equal
// to what VFS do in case setattr operation is NULL.
    return inode_setattr(dentry->d_inode, iattr);
//#endif
}
// Default implementation of file_operations
static inline int
file_operations_open__default(struct inode* inode, struct file* filp)
{
    return 0;
}

static inline int
file_operations_release__default(struct inode* inode, struct file* filp)
{
    return 0;
}

#endif /* FS_OPERATION_REPLACEMENT_H */
