#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
/* ====================================================================== */

#define NUM_LS_DATA_ELEMS 8

struct my_struct {
	/* Address of the function the struct has been created for. */
	void *func;
	
	/* Storage that the pre/post handlers can use. It is preserved 
	 * between the corresponding calls to the pre and post handlers but
	 * may be clobbered after the post handler returns. */
	unsigned long data[NUM_LS_DATA_ELEMS];
	
		
	/* This field can be used by the handlers of indirect calls to store
	 * the address of the called function. */
	void *callee;
	
	/* Some ID. */
	unsigned long pid;
	
};

/* To be used if allocation of the local storage fails. */
static struct my_struct invalid_ls;

struct my_struct *
my_func_dummy_entry(void *func, unsigned int nargs, void **args)
{
	struct my_struct *p;
	unsigned int i;
	
	p = kzalloc(sizeof(struct my_struct), GFP_ATOMIC);
		
	pr_info("[DBG] Function entry, func = %pf, nargs = %u, "
		"my_struct = %p, ret_addr = %p\n",
		func, nargs, p, __builtin_return_address(0));
	
	if (p == NULL) {
		pr_info("my_func_dummy_entry(): out of memory.\n");
		return &invalid_ls;
	}
	
	p->func = func;
	p->pid = (unsigned long)current;
	
	for (i = 0; i < nargs; ++i) {
		pr_info("[DBG]\t#%u: 0x%lx", i, (unsigned long)args[i]);
		if (args[i]) {
			pr_info(" (value: 0x%lx)", *(unsigned long *)args[i]);
		}
		pr_info("\n");
	}
	
	/* [NB] In a real system, we could now use the address of the 
	 * current function ('func') to get additional info. For example, to
	 * lookup the in-function handlers in some global registry which may
	 * be useful for the callbacks like file_operations, and more. */
	
	return p;
}

void
my_func_dummy_exit(struct my_struct *p)
{
	if (p != &invalid_ls) {
		pr_info(
		"[DBG] Function exit, func = %pf, my_struct is at %p\n", 
			p->func, p);
		kfree(p);
	}
	else {
		pr_info("[DBG] Function exit, my_struct is at %p (invalid_ls)\n",
			p);
	}
}
/* ====================================================================== */

/* Handling of memory reads and writes. 
 *
 * pc - address of the instruction somewhere near the place where the event
 * 	occurred;
 * addr - address of the accessed memory area; 
 * size - size of the memory area;
 * is_write - non-zero for writes, 0 for reads;
 * ls - pointer to the local storage. */
static void
report_memory_event(void *pc, void *addr, unsigned int size, int is_write,
		    struct my_struct *ls)
{
	pr_info(
"[DBG] TID=%lx: memory %s at %pS: accessed %u byte(s) starting from %p.\n",
		ls->pid, (is_write ? "write" : "read"), pc, size, addr);
}

void
my_func_read1(void *addr, struct my_struct *ls)
{
	report_memory_event(
		(void *)__builtin_return_address(0), addr, 1, 0, ls);
}

void
my_func_read2(void *addr, struct my_struct *ls)
{
	report_memory_event(
		(void *)__builtin_return_address(0), addr, 2, 0, ls);
}

void
my_func_read4(void *addr, struct my_struct *ls)
{
	report_memory_event(
		(void *)__builtin_return_address(0), addr, 4, 0, ls);
}

void
my_func_read8(void *addr, struct my_struct *ls)
{
	report_memory_event(
		(void *)__builtin_return_address(0), addr, 8, 0, ls);
}

void
my_func_read16(void *addr, struct my_struct *ls)
{
	report_memory_event(
		(void *)__builtin_return_address(0), addr, 16, 0, ls);
}

void
my_func_write1(void *addr, struct my_struct *ls)
{
	report_memory_event(
		(void *)__builtin_return_address(0), addr, 1, 1, ls);
}

void
my_func_write2(void *addr, struct my_struct *ls)
{
	report_memory_event(
		(void *)__builtin_return_address(0), addr, 2, 1, ls);
}

void
my_func_write4(void *addr, struct my_struct *ls)
{
	report_memory_event(
		(void *)__builtin_return_address(0), addr, 4, 1, ls);
}

void
my_func_write8(void *addr, struct my_struct *ls)
{
	report_memory_event(
		(void *)__builtin_return_address(0), addr, 8, 1, ls);
}

void
my_func_write16(void *addr, struct my_struct *ls)
{
	report_memory_event(
		(void *)__builtin_return_address(0), addr, 16, 1, ls);
}
/* ====================================================================== */

/* Function handlers (pre-, post-) and replacement functions. 
 * 
 * Notes for the types:
 * - pointer types remain the same in the handlers;
 * - enum types may remain the same or an appropriate unsigned integer type
 * may be used in the handler;
 * - simple integer types remain the same, so do s64/u64;
 * - if the target has an argument of some other integer type, the 
 * corresponding handler will take it as an unsigned integer type, wide 
 * enough to accomodate the value without truncation (usually unsigned long
 * or unsigned long long);
 * - for each argument of some other type, a copy will be created by the 
 * instrumentation system and the pointer to it will be passed to the 
 * handler. */

/* Handlers for void *vmalloc(unsigned long size) */
void
my_func_vmalloc_pre(unsigned long size, struct my_struct *ls)
{
	pr_info("[DBG] pre handler: vmalloc(%lu), pid: %lx\n", size, ls->pid);
	
	/* Save 'size' argument, pretend the post handler will need it. */
	ls->data[0] = size;
}

/* [NB] If the post handler needs the arguments of the function, the pre 
 * handler should save them somewhere in LS or allocate a structure and 
 * save a pointer to it in LS. */
void
my_func_vmalloc_post(void *ret, struct my_struct *ls)
{
	/* The pre handler must have saved the argument of vmalloc for us
	 * to use. */
	pr_info("[DBG] post handler: vmalloc(%lu) returned %p.\n", 
		ls->data[0], ret);
}

/* Handlers for void vfree(void *addr) */
void
my_func_vfree_pre(void *addr, struct my_struct *ls)
{
	pr_info("[DBG] pre handler: vfree(%p)\n", addr);
	ls->data[0] = (unsigned long)addr;
}

void
my_func_vfree_post(struct my_struct *ls)
{ /* [NB] The target function does not return value. */
	pr_info("[DBG] post handler: vfree(%p)\n",
		(void *)ls->data[0]);	
}

/* Handlers for 
 * int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,
		    const char *name) */
void
my_func_alloc_chrdev_region_pre(
	dev_t *dev, unsigned baseminor, 
	unsigned count, const char *name, struct my_struct *ls)
{
	pr_info("[DBG] pre handler: alloc_chrdev_region(%p, %u, %u, %s)\n",
		dev, baseminor, count, name);
	ls->data[0] = (unsigned long)dev;
	ls->data[1] = baseminor;
	ls->data[2] = count;
	ls->data[3] = (unsigned long)name;
}

void
my_func_alloc_chrdev_region_post(int ret, struct my_struct *ls)
{
	pr_info("[DBG] post handler: alloc_chrdev_region(%p, %u, %u, %s) = %d\n",
	       (void *)ls->data[0],
	       (unsigned)ls->data[1],
	       (unsigned)ls->data[2],
	       (const char *)ls->data[3],
	       ret);	
}

/* Handlers for 
 * void unregister_chrdev_region(dev_t from, unsigned count) 
 * [NB] dev_t will be converted to unsigned long long before the call to the 
 * handler. */
void
my_func_unregister_chrdev_region_pre(unsigned long long from, unsigned count, 
				     struct my_struct *ls)
{
	pr_info("[DBG] pre handler: unregister_chrdev_region(%lu, %u)\n",
		(unsigned long)from, count);	
	ls->data[0] = (unsigned long)from;
	ls->data[1] = count;
}

void
my_func_unregister_chrdev_region_post(struct my_struct *ls)
{
	pr_info("[DBG] post handler: unregister_chrdev_region(%lu, %u)\n",
		ls->data[0], 
	        (unsigned int)ls->data[1]);	
}

/* Handlers for size_t strlen(const char *s) */
void
my_func_strlen_pre(const char *s, struct my_struct *ls)
{
	pr_info("[DBG] pre handler: strlen(%s)\n", s);
	ls->data[0] = (unsigned long)s;
}

void
my_func_strlen_post(unsigned long ret, struct my_struct *ls)
{
	pr_info("[DBG] post handler: strlen(%s) = %lu\n",
		(const char *)ls->data[0], (unsigned long)ret);	
}

/* Handlers for void *memcpy(void *dest, const void *src, size_t count) */
void
my_func_memcpy_pre(void *dest, const void *src, unsigned long count, 
		   struct my_struct *ls)
{
	pr_info("[DBG] pre handler: memcpy(%p, %p, %lu)\n",
		dest, src, count);
	ls->data[0] = (unsigned long)dest;
	ls->data[1] = (unsigned long)src;
	ls->data[2] = count;
}

void
my_func_memcpy_post(void *ret, struct my_struct *ls)
{
	pr_info("[DBG] post handler: memcpy(%p, %p, %lu) = %p\n",
		(void *)ls->data[0],
		(const void *)ls->data[1],
		ls->data[2],
		ret);
}

/* Handlers for void *memset(void *s, int c, size_t count) */
void
my_func_memset_pre(void *s, int c, unsigned long count, 
		   struct my_struct *ls)
{
	pr_info("[DBG] pre handler: memset(%p, %d, %lu)\n",
		s, c, count);
	ls->data[0] = (unsigned long)s;
	ls->data[1] = (unsigned long)c;
	ls->data[2] = count;
}

void
my_func_memset_post(void *ret, struct my_struct *ls)
{
	pr_info("[DBG] post handler: memset(%p, %d, %lu) = %p\n",
		(void *)ls->data[0],
		(int)ls->data[1],
		ls->data[2],
		ret);
}

/* Handlers for int snprintf(char *s, size_t n, const char *format, ...)
 * Only the "fixed" arguments are passed to the handler. */
void
my_func_snprintf_pre(char *s, unsigned long n, const char *format, 
		     struct my_struct *ls)
{
	pr_info("[DBG] pre handler: snprintf(%p, %lu, '%s', ...)\n",
		s, n, format);
	ls->data[0] = (unsigned long)s;
	ls->data[1] = n;
	ls->data[2] = (unsigned long)format;
}

void
my_func_snprintf_post(int ret, struct my_struct *ls)
{
	pr_info("[DBG] post handler: snprintf(%p, %lu, '%s', ...) = %d\n",
		(char *)ls->data[0],
		ls->data[1],
		(const char *)ls->data[2],
		ret);
}

/* Handlers for void * k[mz]alloc(size_t size, unsigned int flags) */
void
my_func_kmalloc_pre(unsigned long size, unsigned int flags, 
		     struct my_struct *ls)
{
	pr_info("[DBG] pre handler: kmalloc(%lu, %x)\n",
		size, flags);
	ls->data[0] = size;
	ls->data[1] = flags;
}

void
my_func_kmalloc_post(void *ret, struct my_struct *ls)
{
	pr_info("[DBG] post handler: kmalloc(%lu, %x) = %p\n",
		ls->data[0],
		(unsigned int)ls->data[1],
		ret);
}

/* Handlers for void kfree(void *addr) */
void
my_func_kfree_pre(void *addr, struct my_struct *ls)
{
	pr_info("[DBG] pre handler: kfree(%p)\n", addr);
	ls->data[0] = (unsigned long)addr;
}

void
my_func_kfree_post(struct my_struct *ls)
{
	pr_info("[DBG] post handler: kfree(%p)\n", (void *)ls->data[0]);
}

/* Handlers for indirect calls. 
 * 
 * One pair of handlers for each group of target functions with compatible
 * signatures. See TypeSeqLessOrEqual.
 * 
 * The address of the target function is in 'callee'. */

/* void *func(unsigned long arg0), e.g. my_alloc() callback */
void
my_func_call_pvoid_ulong_pre(unsigned long arg0, void *callee, 
			     struct my_struct *ls)
{
	if (callee == (void *)vmalloc) {
		pr_info("[DBG] before indirect call to vmalloc(%lu)\n",
			arg0);
	}
	else {
		pr_info("[DBG] before indirect call, func=%pf, args: {%lu}\n",
			callee, arg0);
	}
	
	/* In a real system, we can check the address of the callee here and
	 * call the appropriate handler or do nothing if not found. */
	
	ls->data[0] = arg0;
	ls->callee = callee;
}

void
my_func_call_pvoid_ulong_post(void *ret, struct my_struct *ls)
{
	if (ls->callee == (void *)vmalloc) {
		pr_info("[DBG] after indirect call to vmalloc(%lu), ret=%p\n",
			ls->data[0], ret);
	}
	else {
		pr_info(
		"[DBG] after indirect call, func=%pf, args: {%lu}, ret=%p\n",
			ls->callee, ls->data[0], ret);
	}
}

/* void (*func)(void *arg0, int arg1), e.g. my_free() callback */
void
my_func_call_void_pvoid_int_pre(void *arg0, int arg1, void *callee, 
				struct my_struct *ls)
{
	pr_info("[DBG] before indirect call, func=%pf, args: {%p, %d}\n",
		callee, arg0, arg1);
	
	/* In a real system, we can check the address of the callee here and
	 * call the appropriate handler or do nothing if not found. */
	
	ls->data[0] = (unsigned long)arg0;
	ls->data[1] = (unsigned long)arg1;
	ls->callee = callee;
}

void
my_func_call_void_pvoid_int_post(struct my_struct *ls)
{
	pr_info("[DBG] after indirect call, func=%pf, args: {%p, %d}\n",
		ls->callee, (void *)ls->data[0], (int)ls->data[1]);
}

/* int (*func)(void), e.g. my_get() callback */
void
my_func_call_int_void_pre(void *callee, struct my_struct *ls)
{
	pr_info("[DBG] before indirect call, func=%pf\n", callee);
	
	/* In a real system, we can check the address of the callee here and
	 * call the appropriate handler or do nothing if not found. */
	ls->callee = callee;
}

void
my_func_call_int_void_post(int ret, struct my_struct *ls)
{
	pr_info("[DBG] after indirect call, func=%pf, ret=%d\n",
		ls->callee, ret);
}
/* ====================================================================== */

/* A replacement function for void *__kmalloc(size_t size, gfp_t flags); 
 * 
 * The arguments are the same, except LS is passed as an argument too, at
 * the end of their list. */
void *
my_func___kmalloc_repl(size_t size, unsigned int flags, 
		       struct my_struct *ls)
{
	void *ret;
	
	if (size > 4096) {
		pr_info(
"[DBG] Replacement for __kmalloc(%zu, %x) - simulating a failure.\n",
		size, flags);
		return NULL;
	}
	
	pr_info(
"[DBG] Replacement for __kmalloc(%zu, %x) - before the call, ID = %lx.\n",
		size, flags, ls->pid);
	ret = __kmalloc(size, flags);
	pr_info(
"[DBG] Replacement for __kmalloc(%zu, %x) - after the call (ret = %p).\n",
		size, flags, ret);
	return ret;
}
/* ====================================================================== */
