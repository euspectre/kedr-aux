/*********************************************************************
 * trace_patcher.h: trace-related stuff
 *
 * Based on trace events sample by Steven Rostedt (kernel 2.6.33.1).
 *********************************************************************/

#undef TRACE_SYSTEM
#define TRACE_SYSTEM kedr_patcher

#if !defined(_TRACE_PAYLOAD_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_PAYLOAD_H

/*********************************************************************
 * Definitions of trace events
 *********************************************************************/
TRACE_EVENT(called_capable,

	TP_PROTO(int cap, int returnValue),

	TP_ARGS(cap, returnValue),

	TP_STRUCT__entry(
		__field(int, cap)
		__field(int, returnValue)
	),

	TP_fast_assign(
		__entry->cap = cap;
		__entry->returnValue = returnValue;
	),

	TP_printk("arguments: (%d), result: %d",
		__entry->cap,
		__entry->returnValue
	)
);

TRACE_EVENT(called___kmalloc,

	TP_PROTO(size_t size, unsigned int flags, void* returnValue),

	TP_ARGS(size, flags, returnValue),

	TP_STRUCT__entry(
		__field(size_t, size)
		__field(unsigned int, flags)
		__field(void*, returnValue)
	),

	TP_fast_assign(
		__entry->size = size;
		__entry->flags = flags;
		__entry->returnValue = returnValue;
	),

	TP_printk("arguments: (%zu, %x), result: %p",
		__entry->size,
		__entry->flags,
		__entry->returnValue
	)
);

TRACE_EVENT(called_krealloc,

	TP_PROTO(const void* p, size_t size, unsigned int flags, void* returnValue),

	TP_ARGS(p, size, flags, returnValue),

	TP_STRUCT__entry(
		__field(const void*, p)
		__field(size_t, size)
		__field(unsigned int, flags)
		__field(void*, returnValue)
	),

	TP_fast_assign(
		__entry->p = p;
		__entry->size = size;
		__entry->flags = flags;
		__entry->returnValue = returnValue;
	),

	TP_printk("arguments: (%p, %zu, %x), result: %p",
		__entry->p,
		__entry->size,
		__entry->flags,
		__entry->returnValue
	)
);

TRACE_EVENT(called_kfree,

	TP_PROTO(void* p),

	TP_ARGS(p),

	TP_STRUCT__entry(
		__field(void*, p)
	),

	TP_fast_assign(
		__entry->p = p;
	),

	TP_printk("arguments: (%p)",
		__entry->p
	)
);

TRACE_EVENT(called_kmem_cache_alloc,

	TP_PROTO(void* mc, unsigned int flags, void* returnValue),

	TP_ARGS(mc, flags, returnValue),

	TP_STRUCT__entry(
		__field(void*, mc)
		__field(unsigned int, flags)
		__field(void*, returnValue)
	),

	TP_fast_assign(
		__entry->mc = mc;
		__entry->flags = flags;
		__entry->returnValue = returnValue;
	),

	TP_printk("arguments: (%p, %x), result: %p",
		__entry->mc,
		__entry->flags,
		__entry->returnValue
	)
);

TRACE_EVENT(called_kmem_cache_alloc_notrace,

	TP_PROTO(void* mc, unsigned int flags, void* returnValue),

	TP_ARGS(mc, flags, returnValue),

	TP_STRUCT__entry(
		__field(void*, mc)
		__field(unsigned int, flags)
		__field(void*, returnValue)
	),

	TP_fast_assign(
		__entry->mc = mc;
		__entry->flags = flags;
		__entry->returnValue = returnValue;
	),

	TP_printk("arguments: (%p, %x), result: %p",
		__entry->mc,
		__entry->flags,
		__entry->returnValue
	)
);

TRACE_EVENT(called_kmem_cache_free,

	TP_PROTO(void* mc, void* p),

	TP_ARGS(mc, p),

	TP_STRUCT__entry(
		__field(void*, mc)
		__field(void*, p)
	),

	TP_fast_assign(
		__entry->mc = mc;
		__entry->p = p;
	),

	TP_printk("arguments: (%p, %p)",
		__entry->mc,
		__entry->p
	)
);

TRACE_EVENT(called___mutex_init,

	TP_PROTO(struct mutex* lock, const char* name, struct lock_class_key* key),

	TP_ARGS(lock, name, key),

	TP_STRUCT__entry(
		__field(struct mutex*, lock)
		__field(const char*, name)
		__field(struct lock_class_key*, key)
	),

	TP_fast_assign(
		__entry->lock = lock;
		__entry->name = name;
		__entry->key = key;
	),

	TP_printk("arguments: (%p, %p, %p)",
		__entry->lock,
		__entry->name,
		__entry->key
	)
);

TRACE_EVENT(called_mutex_lock,

	TP_PROTO(struct mutex* lock),

	TP_ARGS(lock),

	TP_STRUCT__entry(
		__field(struct mutex*, lock)
	),

	TP_fast_assign(
		__entry->lock = lock;
	),

	TP_printk("arguments: (%p)",
		__entry->lock
	)
);

TRACE_EVENT(called_mutex_lock_interruptible,

	TP_PROTO(struct mutex* lock, int returnValue),

	TP_ARGS(lock, returnValue),

	TP_STRUCT__entry(
		__field(struct mutex*, lock)
		__field(int, returnValue)
	),

	TP_fast_assign(
		__entry->lock = lock;
		__entry->returnValue = returnValue;
	),

	TP_printk("arguments: (%p), result: %d",
		__entry->lock,
		__entry->returnValue
	)
);

TRACE_EVENT(called_mutex_trylock,

	TP_PROTO(struct mutex* lock, int returnValue),

	TP_ARGS(lock, returnValue),

	TP_STRUCT__entry(
		__field(struct mutex*, lock)
		__field(int, returnValue)
	),

	TP_fast_assign(
		__entry->lock = lock;
		__entry->returnValue = returnValue;
	),

	TP_printk("arguments: (%p), result: %d",
		__entry->lock,
		__entry->returnValue
	)
);

TRACE_EVENT(called_mutex_unlock,

	TP_PROTO(struct mutex* lock),

	TP_ARGS(lock),

	TP_STRUCT__entry(
		__field(struct mutex*, lock)
	),

	TP_fast_assign(
		__entry->lock = lock;
	),

	TP_printk("arguments: (%p)",
		__entry->lock
	)
);

TRACE_EVENT(called__raw_spin_lock_irqsave,

	TP_PROTO(raw_spinlock_t* lock, unsigned long returnValue),

	TP_ARGS(lock, returnValue),

	TP_STRUCT__entry(
		__field(raw_spinlock_t*, lock)
		__field(unsigned long, returnValue)
	),

	TP_fast_assign(
		__entry->lock = lock;
		__entry->returnValue = returnValue;
	),

	TP_printk("arguments: (%p), result: %lu",
		__entry->lock,
		__entry->returnValue
	)
);

TRACE_EVENT(called__raw_spin_unlock_irqrestore,

	TP_PROTO(raw_spinlock_t* lock, unsigned long flags),

	TP_ARGS(lock, flags),

	TP_STRUCT__entry(
		__field(raw_spinlock_t*, lock)
		__field(unsigned long, flags)
	),

	TP_fast_assign(
		__entry->lock = lock;
		__entry->flags = flags;
	),

	TP_printk("arguments: (%p, %lu)",
		__entry->lock,
		__entry->flags
	)
);

TRACE_EVENT(called_copy_to_user,

	TP_PROTO(void __user* to, void* from, unsigned long n, unsigned long returnValue),

	TP_ARGS(to, from, n, returnValue),

	TP_STRUCT__entry(
		__field(void __user*, to)
		__field(void*, from)
		__field(unsigned long, n)
		__field(unsigned long, returnValue)
	),

	TP_fast_assign(
		__entry->to = to;
		__entry->from = from;
		__entry->n = n;
		__entry->returnValue = returnValue;
	),

	TP_printk("arguments: (%p, %p, %lu), result: %lu",
		__entry->to,
		__entry->from,
		__entry->n,
		__entry->returnValue
	)
);

TRACE_EVENT(called__copy_from_user,

	TP_PROTO(void* to, void __user* from, unsigned long n, unsigned long returnValue),

	TP_ARGS(to, from, n, returnValue),

	TP_STRUCT__entry(
		__field(void*, to)
		__field(void __user*, from)
		__field(unsigned long, n)
		__field(unsigned long, returnValue)
	),

	TP_fast_assign(
		__entry->to = to;
		__entry->from = from;
		__entry->n = n;
		__entry->returnValue = returnValue;
	),

	TP_printk("arguments: (%p, %p, %lu), result: %lu",
		__entry->to,
		__entry->from,
		__entry->n,
		__entry->returnValue
	)
);
/*********************************************************************/
#endif /* _TRACE_PAYLOAD_H */

/* This is for the trace events machinery to be defined properly */
#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_PATH .

#define TRACE_INCLUDE_FILE trace_patcher
#include <trace/define_trace.h>
