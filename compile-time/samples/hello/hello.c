/* gcc -o hello -g -O2 -fplugin=<path_to_pligin> hello.c my_funcs.o stubs.o
 * -fdump-tree-einline-raw may be worth adding too to see the resulting IR.
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* ====================================================================== */

/* Declarations of the stubs. */
void *
vmalloc(unsigned long size);

void 
vfree(void *addr);

int 
alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,
		    const char *name);
void 
unregister_chrdev_region(dev_t from, unsigned count);

void *
__kmalloc(size_t size, unsigned int flags);

void 
kfree(void *addr);
/* ====================================================================== */

typedef enum {
	EVALUE_A = 1,
	EVALUE_B,
	EVALUE_C,
	EVALUE_MAX
} MyEnum;

struct my_ops {
	void *(*my_alloc)(unsigned long size);
	void (*my_free)(void *addr, short int somedata);
	int (*my_get)();
	MyEnum (*my_other)(int unused);
};

static void *
dummy_alloc(unsigned long size)
{
	return NULL;
}

static void 
dummy_free(void *addr, short int somedata)
{
	return;	
}

static int
dummy_get(void)
{
	return 42;	
}

static void 
real_free(void *addr, short int somedata)
{
	vfree(addr);
	return;	
}

static int
real_get(void)
{
	return 24;
}

static MyEnum
real_other(int unused)
{
	(void)unused;
	return EVALUE_C;
}

static struct my_ops dummy_ops = {
	.my_alloc = dummy_alloc,
	.my_free = dummy_free,
	.my_get = dummy_get,
};

static struct my_ops real_ops = {
	.my_alloc = vmalloc,
	.my_free = real_free,
	.my_get = real_get,
	.my_other = real_other,
};

void *my_area = NULL;
/* ====================================================================== */

struct my_struct1 {
	unsigned long a;
	unsigned long b;
	unsigned long c;
	void *p;
};

static void *
kmalloc(size_t size, unsigned int flags)
{
	return __kmalloc(size, flags);
}

static void *
kzalloc(size_t size, unsigned int flags)
{
	return __kmalloc(size, flags | 0x4000);
}

static int
other_func(const char *p, int x, unsigned long u, struct my_struct1 s, 
	unsigned long other[], struct my_ops *ops) 
{
	void *ptr;
	void *t;
	char str[64];
	size_t len;
	int nchars;
	
	printf("x = %d, p = %s, u = %lu, args: %lx, %lx\n", 
		x, p, u, other[0], other[1]);
	printf("%lu\n", (unsigned long )s.b);
	
	my_area = ops->my_alloc(10);
	
	s.b = s.a + other[0];
	ptr = vmalloc(57);
	*(char *)ptr = p[0];
	vfree(ptr);
	s.c = other[0] + other[1];
	
	len = strlen(p) + 1;
	if (len > sizeof(str))
		return -1;
	
	s.b += other[1];
	memset(str, 0, sizeof(str));
	memcpy(str, p, len);
	nchars = snprintf(str, sizeof(str), "***test: %s, x=%d***", p, x);
	
	printf("str: %s, nchars: %d\n", str, nchars);
	printf("some value: %d\n", ops->my_get());
	
	/* A call with an ignored return value. */
	ops->my_get();
	
	ops->my_free(my_area, (short int)x/*0x0123deed*/);
	my_area = NULL;
	
	my_area = kzalloc(100, 0x8afe);
	if (!my_area) {
		printf("kzalloc() failed!\n");
		return -1;
	}
	kfree(my_area);
	
	my_area = kmalloc(100, 0x8afe);
	if (!my_area) {
		printf("kmalloc() failed!\n");
		return -1;
	}
	kfree(my_area);
	
	if (ops->my_other)
		printf("[DBG] my_other returned %d\n", (int)ops->my_other(55));
	
	return x + (int)u + (int)s.b;
}

static void
argless_func(void)
{
	printf("Test.\n");
}

int
main(int argc, char *argv[])
{
	int *p = &argc;
	int **pp = &p;
	unsigned long args[8];
	
	printf("%p\n", pp);
	args[0] = 4;
	args[1] = (unsigned long)pp;
	
	if (argc > 2) {
		struct my_struct1 s;
		int ret;
		dev_t dev;
		
		s.a = 10;
		s.b = 20;
		s.c = 40;
		
		ret = alloc_chrdev_region(&dev, 0, 3, "cfake");
		if (ret > 0)
			return ret;
		
		other_func("something", argc - 1, (unsigned long)&main, s, 
			args, (argc == 4 ? &real_ops : &dummy_ops));
		printf("Hello, Hacker!\n");
		unregister_chrdev_region(dev, 3);
	}
	else {
		argless_func();
		printf("Hello, World!\n");
	}
	return 0;
}
