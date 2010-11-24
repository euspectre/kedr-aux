/*
 * Example of LTT probe module for KEDR, based on ltt/probes/mm-trace.c
 */

#include <linux/module.h>
#include <linux/ltt-type-serializer.h>

/* probe for __kmalloc replacement in KEDR*/
#include <payloads_callm/common_memory_management/trace_payload.h>
void probe_called___kmalloc(void* abs_addr, int section_id,
	ptrdiff_t rel_addr, size_t size, unsigned int flags, void* returnValue);

DEFINE_MARKER_TP(kedr_cmm, called___kmalloc, called___kmalloc, probe_called___kmalloc,
	"bytes_alloc %zu flags %x result %p");

struct serialize_alloc {
	size_t size;
	unsigned int flags;
	void* result;
	unsigned char end_field[0];
} LTT_ALIGN;

notrace void probe_called___kmalloc(void* abs_addr, int section_id,
	ptrdiff_t rel_addr, size_t size, unsigned int flags, void* returnValue)
{
	struct marker *marker;
	struct serialize_alloc data;
	
	data.size = size;
	data.flags = flags;
	data.result = returnValue;
	
	marker = &GET_MARKER(kedr_cmm, called___kmalloc);
	ltt_specialized_trace(marker, marker->single.probe_private,
		&data, serialize_sizeof(data), sizeof(long));
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tsyvarev");
MODULE_DESCRIPTION("Tracepoint Probes Test");
