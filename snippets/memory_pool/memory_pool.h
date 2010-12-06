/*
 * Provide wrapper to allocator, 
 * which control in some extent access to allocated memory.
 *
 * Such control has features:
 * 
 * 1) Prevent access to the memory after the bound of allocated one.
 * 2) Prevent access to the recently freed memory
 *     (until some period of time, or some number of allocation 
 * 	   occures after free)
 * 3) Allocated memory is filled with some "garbage",
 * 	   so reading of uninitializing memory may lead to error.
 */

#include <linux/slab.h>

typedef struct mempool_allocator* mempool_allocator_t;

/*
 * Operations of allocator, for which wrapper is created.
 */
struct mempool_allocator_ops
{
	/*
	 * Should allocate contiguous memory of size 'size'.
	 * 
	 * 'data' is same as given to the wrapper allocation function.
	 */ 
	void* (*alloc)(size_t size, void* data);
	void (*free)(void* addr);
	
	/*
	 * Optional functions.
	 * 
	 * If not NULL, them will be used by wrapper.
	 * Otherwise them will be emulated.
	 */
	
	/*
	 * Should allocate memory of size at least as 'size',
	 * and with right boundary aligned on PAGE_SIZE.
	 * 
	 * Return value is interpreted as start of allocated memory, which
	 * may be passed to free().
	 * 
	 * (<return value> + size - 1) is interpreted as pointer inside
	 * the last page, which is FULLY covered by the allocated memory.
	 * 
	 * (That means, that caller may extend (<return value> + size) 
	 * memory boundary to the page boundary.)
	 */
	void* (*alloc_with_right_alignment)(size_t size, void* data);
};

/*
 * Create memory pool allocator, which may be used for allocate memory
 * with definite controlling.
 */

mempool_allocator_t
mempool_allocator_create(struct mempool_allocator_ops* ops);

/*
 * Destroy memory pool allocator
 */

void mempool_allocator_destroy(mempool_allocator_t allocator);

/*
 * Allocate memory of size 'size'.
 * 
 * 'data' parameter will be passed to the corresponding 
 * allocator function, for which this allocator was created.
 */
void* mempool_allocator_alloc(mempool_allocator_t allocator,
	size_t size, void* data);

/*
 * Free memory, allocated by the 'allocator'.
 *
 * Return 0.
 * 
 * If given memory wasn't allocated by this allocator,
 * do nothing and return not 0.
 */
int mempool_allocator_free(mempool_allocator_t allocator,
	void* addr);

/*
 * Return size of the allocated object.
 * 
 * If object was not allocated by this allocator, return 0.
 */
size_t mempool_allocator_size(mempool_allocator_t allocator,
	void* addr);
