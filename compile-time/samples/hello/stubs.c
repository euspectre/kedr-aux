/* Stubs imitating the external functions to test handling of the calls to
 * such functions.
 *
 * gcc -c -o stubs.o stubs.c */

#include <stdio.h>
#include <stdlib.h>

void *
vmalloc(unsigned long size)
{
	printf("[STUB] vmalloc(%lu)\n", size);
	return malloc(size);
}

void 
vfree(void *addr)
{
	printf("[STUB] vfree(%p)\n", addr);
	free(addr);
}

int 
alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,
		    const char *name)
{
	printf("[STUB] alloc_chrdev_region(%p, %u, %u, %s)\n",
		dev, baseminor, count, name);
	*dev = 0x12;
	return 0; /* success */
}

void 
unregister_chrdev_region(dev_t from, unsigned count)
{
	printf("[STUB] unregister_chrdev_region(%lu, %u)\n",
		(unsigned long)from, count);
}

void *
__kmalloc(size_t size, unsigned int flags)
{
	printf("[STUB] __kmalloc(%lu, %x)\n", (unsigned long)size, flags);
	return malloc(size);
}

void 
kfree(void *addr)
{
	printf("[STUB] kfree(%p)\n", addr);
	free(addr);
}
