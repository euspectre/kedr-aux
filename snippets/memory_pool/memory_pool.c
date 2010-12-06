#include "memory_pool.h"

#include <linux/list.h> /* Use list of blocks*/
#include <linux/spinlock.h> /* Use spinlock for protect list of blocks */

//#include <linux/page-flags.h> /* Use 'error' flag for block page*/


//#include <asm/cacheflush.h> /* set_page_*() */

#include <linux/mm.h> /* for blocking page*/
#include <linux/sched.h> /* current */

//#include <linux/bootmem.h> /*max_low_pfn*/
#include <asm/pgtable.h>
#include <asm/pgtable_types.h>
#include <linux/hardirq.h> /* in_irq in pte_offset_map*/

#include <asm/io.h> /*virt_to_phys*/
static void dump_pagetable(unsigned long address)
{
    pgd_t *pgd;
    pud_t *pud;
    pmd_t* pmd;
    pte_t pte, *ppte;

    pgd_t* pgd_addr;
#ifdef CONFIG_X86
    pgd_addr = (pgd_t*)(__va(/*read_cr3()*/0x100000 & PHYSICAL_PAGE_MASK));
#else
#error Cannot determine adress of the global page table on non-x86 architecure.
#endif

    ((int*)address)[0] = 0;
    pr_info("PGD base(%p).", pgd_addr);
    pgd = pgd_addr + pgd_index(address);
    //pgd = pgd_offset(current->mm, address);
    pr_info("PGD(%p) is 0x%lx.", pgd, pgd_val(*pgd));
    if(pgd_bad(*pgd))
    {
        pr_info("PGD address is bad.");
        return;
    }
    if(pgd_none(*pgd))
    {
        pr_info("PGD is not present.");
        return;
    }

    pud = pud_offset(pgd, address);
    pr_info("PUD(%p) is 0x%lx.", pud, pud_val(*pud));
    if(pud_bad(*pud))
    {
        pr_info("PUD address is bad.");
        return;
    }
    if(pud_none(*pud))
    {
        pr_info("PUD is not present.");
        return;
    }
    pmd = pmd_offset(pud, address);
    pr_info("PMD(%p) is 0x%lx.", pmd, pmd_val(*pmd));
    if(pmd_bad(*pmd))
    {
        pr_info("PMD address is bad.");
        //return;
    }
    if(pmd_none(*pmd))
    {
        pr_info("PMD is not present.");
        return;
    }

    ppte = pte_offset_kernel(pmd, address);
    pr_info("PTE(%p).", ppte);
    if(!virt_addr_valid(ppte))
    {
        pr_info("PTE address is bad.");
        return;
    }
    
    pte = *ppte;
    
    pr_info("Virtual addr: %p, physical address: 0x%lx, last flags: %c%c%c.\n",
            __va(pte_val(pte) & PAGE_MASK)/*address*/,
            pte_val(pte) & PAGE_MASK,
            pte_flags(pte) & _PAGE_USER ? 'U' : 'K',
            pte_flags(pte) & _PAGE_RW ? 'W' : 'R',
            pte_flags(pte) & _PAGE_PRESENT ? 'P' : '-');
    pr_info("Real virtual address: %p, real physical address: 0x%lx.\n",
            (void*)address,
            (unsigned long)virt_to_phys((void*)address));
}


static int page_no_access(void* addr)
{
    dump_pagetable((unsigned long)addr);
    return 0;
}
static void page_restore_access(void* addr)
{
    dump_pagetable((unsigned long)addr);
};

/*
 * Allocated object structure:
 * 
 *
 *   |block_head  |padding|... for usage by the caller ...| page with no access | tail of allocated memory |
 *   |                                                    |
 *  address returned by allocator                     page_boundary
 * 
 * 
 */

struct mempool_block
{
    size_t size;
    struct list_head list;
};

//Return alignment for data of given size
static size_t
mempool_get_alignment(size_t size)
{
    //approximately
    if(size >= 16) return 16;
    else if(size >= 8) return 8;
    else if(size >= 4) return 4;
    else if(size >= 2) return 2;
    return 1;
}
//Auxiliary functions
/*
 * |..obj of size 'size'...| padding|
 * |                                |
 * alignment 'align'            page boundary
 * |...........size_aligned.........|
 * 
 * (align <= PAGE_SIZE)
 */
static size_t mempool_get_size_aligned(size_t size, size_t align)
{
    size_t result = ((size - 1) & ~(align - 1)) + align;
    //pr_debug("mempool_get_size_aligned: arguments: (0x%zx, 0x%zx), return 0x%zx.",
    //    size, align, result);
    return result;
}

// Return address of the page, which contain given address
static inline void* page_addr(void* addr)
{
    void* result =
        (void*)((ptrdiff_t)addr & PAGE_MASK);
    //pr_debug("page_addr: argument: (%p), return %p.", addr, result);
    return result;
}
//Return relative address on the page
static inline ptrdiff_t page_addr_rel(void* addr)
{
    ptrdiff_t result = (ptrdiff_t)addr & ~(ptrdiff_t)PAGE_MASK;
    //pr_debug("page_addr_rel: argument: (%p), return 0x%tx.", addr, result);
    return result;
}

// Adress, which will be returned to the caller by .._alloc().
static void*
mempool_block_get_address(struct mempool_block* block)
{
    //address of the allocated memory on the page
    size_t obj_size = mempool_get_size_aligned(block->size,
            mempool_get_alignment(block->size));
    ptrdiff_t rel_addr = page_addr_rel((void*)(-obj_size));
    //address of the first page of the allocated memory
    void* first_page_addr = page_addr(block);
    ptrdiff_t block_rel_addr = page_addr_rel(block);
    if(block_rel_addr + sizeof(*block) >= rel_addr)
    {
        //allocated memory starts on the next page
        first_page_addr += PAGE_SIZE;
    }
    return first_page_addr + rel_addr;
}
// Size of the allocated memory, which may be used by the caller.
static int
mempool_block_get_size(struct mempool_block* block)
{
    return block->size;
}
// Page which is locked.
static void*
mempool_block_get_address_locked(struct mempool_block* block)
{
    return page_addr(mempool_block_get_address(block)
        + mempool_block_get_size(block) - 1) + PAGE_SIZE;
}
// Address, which should be passed to the free() for freeing memory.
static void*
mempool_block_get_address_for_free(struct mempool_block* block)
{
    return (void*) block;
}


struct mempool_allocator
{
    struct mempool_allocator_ops* ops;
    struct list_head blocks;
    spinlock_t lock;//protect 'blocks' from concurrent reads and writes
};

// Wrapper for allocator function.
static void*
mempool_alloc_with_right_alignment(struct mempool_allocator_ops *ops,
    size_t size, void* data)
{
    if(ops->alloc_with_right_alignment)
        return ops->alloc_with_right_alignment(size, data);
    else return ops->alloc(size + PAGE_SIZE - 1, data);
}


/*
 * Create memory pool allocator, which may be used for allocate memory
 * with definite controlling.
 */

mempool_allocator_t
mempool_allocator_create(struct mempool_allocator_ops* ops)
{
    struct mempool_allocator* allocator =
        kmalloc(sizeof(*allocator), GFP_KERNEL);
    
    if(allocator == NULL)
    {
        pr_err("Cannot allocate structure for memory pool.");
        return NULL;
    }

    allocator->ops = ops;
    INIT_LIST_HEAD(&allocator->blocks);
    spin_lock_init(&allocator->lock);
    pr_debug("PAGE_SIZE is 0x%x.", (int)PAGE_SIZE);
    return allocator;
}

/*
 * Destroy memory pool allocator
 */

void mempool_allocator_destroy(mempool_allocator_t allocator)
{
    BUG_ON(!list_empty(&allocator->blocks));
    kfree(allocator);
}

/*
 * Allocate memory of size 'size'.
 * 
 * 'data' parameter will be passed to the corresponding 
 * allocator function, for which this allocator was created.
 */
void* mempool_allocator_alloc(mempool_allocator_t allocator,
    size_t size, void* data)
{
    unsigned long flags;
    struct mempool_block* block;
    size_t real_size = sizeof(struct mempool_block)
        + mempool_get_size_aligned(size, mempool_get_alignment(size))
        + PAGE_SIZE;
    block = mempool_alloc_with_right_alignment(allocator->ops,
        real_size, data);
    if(block == NULL)
    {
        pr_err("Cannot allocate memory for block.");
        return NULL;
    }
    block->size = size;
    page_no_access(mempool_block_get_address_locked(block));
    pr_debug("Block address: %p.", block);
    pr_debug("Returned memory address: %p, size: %zu.",
        mempool_block_get_address(block),
        mempool_block_get_size(block));
    pr_debug("Address of the page blocked: %p.",
        mempool_block_get_address_locked(block));
    //((char*)mempool_block_get_address_locked(block))[0] = 'f';//should fail
    spin_lock_irqsave(&allocator->lock, flags);
    list_add(&block->list, &allocator->blocks);
    spin_unlock_irqrestore(&allocator->lock, flags);
    
    return mempool_block_get_address(block);
}

int mempool_allocator_free(mempool_allocator_t allocator,
    void* addr)
{
    unsigned long flags;
    struct mempool_block* block;
    spin_lock_irqsave(&allocator->lock, flags);
    list_for_each_entry(block, &allocator->blocks, list)
    {
        if(mempool_block_get_address(block) == addr)
        {
            list_del(&block->list);
            spin_unlock_irqrestore(&allocator->lock, flags);
            page_restore_access(mempool_block_get_address_locked(block));
            allocator->ops->free(
                mempool_block_get_address_for_free(block));
            return 0;
        }
    }
    spin_unlock_irqrestore(&allocator->lock, flags);
    return 1;
}

/*
 * Return size of the allocated object.
 * 
 * If object was not allocated by this allocator, return 0.
 */
size_t mempool_allocator_size(mempool_allocator_t allocator,
    void* addr)
{
    unsigned long flags;
    struct mempool_block* block;
    size_t result = 0;
    spin_lock_irqsave(&allocator->lock, flags);
    list_for_each_entry(block, &allocator->blocks, list)
    {
        if(mempool_block_get_address(block) == addr)
        {
            result = mempool_block_get_size(block);
            break;
        }
    }
    spin_unlock_irqrestore(&allocator->lock, flags);
    return result;
}
