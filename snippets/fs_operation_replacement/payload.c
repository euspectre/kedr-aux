#include <linux/module.h>
#include <linux/kernel.h> /*printk*/

#include "fs_operation_replacement.h"

#include <linux/mount.h> /*struct vfsmnt*/

/*
 * Example of using operation replacement mechanism for VFS elements.
 */

MODULE_AUTHOR("Tsyvarev Andrey");
MODULE_LICENSE("GPL");

static int fst_get_sb__repl(struct file_system_type *fst,
	int flags, const char *dev_name, void *raw_data,
	struct vfsmount * mnt)
{
	int result;
	typeof(fst_get_sb__repl)* get_sb__orig =
		file_system_type_get_orig(get_sb, fst);
	pr_info("Intercept file_system_type::get_sb() operation (caller object is %p).",
		fst);

	result = get_sb__orig
		? get_sb__orig(fst, flags, dev_name, raw_data, mnt)
		: file_system_type_get_sb__default(fst, flags, dev_name, raw_data, mnt);
	if(result) return result;
	
	return 0;
}

static void fst_kill_sb__repl(struct super_block *super)
{
	typeof(fst_kill_sb__repl)* kill_sb__orig =
		file_system_type_get_orig(kill_sb, super->s_type);
	pr_info("Intercept file_system_type::kill_sb() operation (caller object is %p).",
		super->s_type);

	kill_sb__orig
		? kill_sb__orig(super)
		: file_system_type_kill_sb__default(super);
}

static int iops_mkdir__repl(struct inode* inode, struct dentry* dentry, int mode)
{
	typeof(iops_mkdir__repl)* mkdir__orig =
		inode_operations_get_orig(mkdir, inode);

	pr_info("Intercept inode_operations::mkdir() operation (caller inode is %p).",
		inode);

	return mkdir__orig
		? mkdir__orig(inode, dentry, mode)
		: inode_operations_mkdir__default(inode, dentry, mode);
}

static int iops_create__repl(struct inode *dir,
	struct dentry *dentry, int mode, struct nameidata *nd)
{
	typeof(iops_create__repl)* create__orig =
		inode_operations_get_orig(create, dir);

	pr_info("Intercept inode_operations::create() operation (caller inode is %p).",
		dir);

	return create__orig
		? create__orig(dir, dentry, mode, nd)
		: inode_operations_create__default(dir, dentry, mode, nd);
}

static int iops_permission__repl(struct inode *inode, int mask)
{
	typeof(iops_permission__repl)* permission__orig =
		inode_operations_get_orig(permission, inode);

	pr_info("Intercept inode_operations::permission() operation (caller inode is %p).",
		inode);

	return permission__orig
		? permission__orig(inode, mask)
		: inode_operations_permission__default(inode, mask);
}

static int iops_getattr__repl(struct vfsmount *mnt, 
	struct dentry *dentry, struct kstat *stat)
{
	struct inode* inode = dentry->d_inode;

	typeof(iops_getattr__repl)* getattr__orig =
		inode_operations_get_orig(getattr, inode);

	pr_info("Intercept inode_operations::getattr() operation (caller inode is %p).",
		inode);

	return getattr__orig
		? getattr__orig(mnt, dentry, stat)
		: inode_operations_getattr__default(mnt, dentry, stat);
}

static struct inode* sops_alloc_inode__repl(struct super_block* super)
{
	struct inode* inode;
	typeof(sops_alloc_inode__repl)* alloc_inode__orig =
		super_operations_get_orig(alloc_inode, super);
	inode = alloc_inode__orig
		? alloc_inode__orig(super)
		: super_operations_alloc_inode__default(super);

	if(inode)
		pr_info("New inode was allocated (%p).", inode);
	
	return inode;
}

static void sops_destroy_inode__repl(struct inode* inode)
{
	typeof(sops_destroy_inode__repl)* destroy_inode__orig =
		super_operations_get_orig(destroy_inode, inode->i_sb);
	
	destroy_inode__orig
		? destroy_inode__orig(inode)
		: super_operations_destroy_inode__default(inode);
	
	pr_info("Inode (%p) was destroyed.", inode);
}

static int file_operations_open__repl(struct inode* inode, struct file* filp)
{
	const char* file_type;
	int inode_mode = inode->i_mode;

	typeof(file_operations_open__repl)* open__orig =
		file_operations_get_orig(open, filp);

	if(S_ISLNK(inode_mode))
		file_type = "link";
	else if(S_ISREG(inode_mode))
		file_type = "regular file";
	else if(S_ISDIR(inode_mode))
		file_type = "directory";
	else if(S_ISCHR(inode_mode))
		file_type = "character device";
	else if(S_ISBLK(inode_mode))
		file_type = "block device";
	else if(S_ISFIFO(inode_mode))
		file_type = "fifo";
	else if(S_ISSOCK(inode_mode))
		file_type = "socket";
	else
		file_type = "unknown";

	pr_info("Intercept file_operations::open(). File type is %s.", file_type);
	
	return open__orig
		? open__orig(inode, filp)
		: file_operations_open__default(inode, filp);
}

static void on_target_load(struct module* m)
{
	pr_info("User payload become used now.");
}

static void on_target_unload(struct module* m)
{
	pr_info("User payload become unused now.");
}

static struct file_system_type fst_repl =
{
	.get_sb = fst_get_sb__repl,
	.kill_sb = fst_kill_sb__repl
};

static struct file_system_type fst_mask =
{
	.get_sb = REPLACEMENT_MASK,
	.kill_sb = REPLACEMENT_MASK
};

static struct operation_payload fst_payload =
{
	.m = THIS_MODULE,
	.repl = &fst_repl,
	.mask = &fst_mask,
	.target_load_callback = on_target_load,
	.target_unload_callback = on_target_unload
};


static struct super_operations sops_repl =
{
	.alloc_inode = sops_alloc_inode__repl,
	.destroy_inode = sops_destroy_inode__repl,
};

static struct super_operations sops_mask =
{
	.alloc_inode = REPLACEMENT_MASK,
	.destroy_inode = REPLACEMENT_MASK,
};

static struct operation_payload sops_payload =
{
	.m = THIS_MODULE,
	.repl = &sops_repl,
	.mask = &sops_mask,
};

static struct inode_operations iops_repl =
{
	.mkdir = iops_mkdir__repl,
	.create = iops_create__repl,
	.permission = iops_permission__repl,
	.getattr = iops_getattr__repl,
};

static struct inode_operations iops_mask =
{
	.mkdir = REPLACEMENT_MASK,
	.create = REPLACEMENT_MASK,
	.permission = REPLACEMENT_MASK,
	.getattr = REPLACEMENT_MASK,
};

static struct operation_payload iops_payload =
{
	.m = THIS_MODULE,
	.repl = &iops_repl,
	.mask = &iops_mask
};

struct file_operations fops_repl =
{
	.open = file_operations_open__repl,
};

struct file_operations fops_mask =
{
	.open = REPLACEMENT_MASK,
};

static struct operation_payload fops_payload =
{
	.m = THIS_MODULE,
	.repl = &fops_repl,
	.mask = &fops_mask,
};
/* ================================================================ */
static int __init
replacer_init(void)
{
	int result = fs_operation_replacement_init();
	if(result) goto err_fs_init;
	result = file_system_type_payload_register(&fst_payload);
	if(result) goto err_fst_payload;
	result = super_operations_payload_register(&sops_payload);
	if(result) goto err_sops_payload;
  	result = inode_operations_payload_register(&iops_payload);
	if(result) goto err_iops_payload;
  	result = file_operations_payload_register(&fops_payload);
	if(result) goto err_fops_payload;
    
	return 0;

err_fops_payload:
	inode_operations_payload_unregister(&iops_payload);
err_iops_payload:
	super_operations_payload_unregister(&sops_payload);
err_sops_payload:
	file_system_type_payload_unregister(&fst_payload);
err_fst_payload:
	fs_operation_replacement_destroy();
err_fs_init:
	return result;
}

static void
replacer_exit(void)
{
    file_operations_payload_unregister(&fops_payload);
	inode_operations_payload_unregister(&iops_payload);
	super_operations_payload_unregister(&sops_payload);
	file_system_type_payload_unregister(&fst_payload);
    fs_operation_replacement_destroy();
    return;
}

module_init(replacer_init);
module_exit(replacer_exit);
