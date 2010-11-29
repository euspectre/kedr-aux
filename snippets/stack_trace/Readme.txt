This example presents kedr_save_stack_trace() - a helper macro - and 
kedr_save_stack_trace_impl() function it uses. This snippet is based on the 
sample target module from KEDR.
========================================================================

Description

kedr_save_stack_trace() allows to obtain a portion of the stack trace above 
the point of call. That is, the first entry of the obtained stack trace 
corresponds to the location following the call to the function that called 
kedr_save_stack_trace(). The second entry correponds to the caller of the 
function that contains that location, etc.

In particular, if kedr_save_stack_trace() is called from a replacement 
function in a payload module for KEDR, the first entry corresponds to the 
return address of that replacement function (i.e., answers the question 
"Who called me?").

The parameters of kedr_save_stack_trace() and kedr_save_stack_trace_impl() 
are described in the comments in cfake.c.

[NB] Unlike kedr_save_stack_trace(), save_stack_trace() kernel function may 
return the stack entries 'below' the point of call. These addresses 
correspond to dump_trace(), show_trace(), save_stack_trace() itself, etc. 
We are not often interested in these in the payload modules.

Declarations:
void
kedr_save_stack_trace_impl(unsigned long *entries, unsigned int max_entries,
    unsigned int *nr_entries,
    unsigned long first_entry);

#define kedr_save_stack_trace(entries_, max_entries_, nr_entries_) <...>
The parameters of kedr_save_stack_trace() and kedr_save_stack_trace_impl() 
are described in the comments in cfake.c.

[NB] kedr_save_stack_trace() will return at least one entry even if 
save_stack_trace() is a no-op (e.g. if CONFIG_STACKTRACE is not defined). 
It relies on __builtin_return_address() in this case.

[NB] kedr_save_stack_trace() works even if CONFIG_FRAME_POINTER is not 
defined, that is, if frame pointer omission is active (like it is on 
OpenSuSE 11.3 by default).

This example also demonstrates automatic resolution of call addresses. 
printf-like functions in the kernel support "%pS" specifier for (void *)
pointers. If it is used, the following will be printed for a given address:
<function>+<offset>/<function_size>

For example, 
	printk (KERN_INFO "%pS\n", (void *)(0xc02fced4))
may print something like this:
	chrdev_open+0xb4/0x1a0 

Here, "chrdev_open" is the name of the function (or, in general, of the 
symbol) where the specified address belongs, 0xb4 is the offset from the 
beginning of that function, 0x1a0 - the length of that function (in bytes).
========================================================================

Building and Testing

To build the example, just execute 'make'.

kedr_sample_target.ko module will be built.

To test the example, you can use the following commands (as root):
# ./kedr_sample_target load
# echo 12345 > /dev/cfake
# ./kedr_sample_target unload

After that, the system log will contain several records like the following 
ones:
----------------------------
[cr_target] open(), stack entries: 5
[cr_target] stack entry #0: [<c02fced4>] chrdev_open+0xb4/0x1a0
[cr_target] stack entry #1: [<c02f7895>] __dentry_open+0xc5/0x280
[cr_target] stack entry #2: [<c02f8e71>] nameidata_to_filp+0x51/0x70
[cr_target] stack entry #3: [<c0304fcf>] finish_open+0xbf/0x1a0
[cr_target] stack entry #4: [<c03057b2>] do_filp_open+0x1e2/0x550
[cr_target] == Cleaning up ==
[cr_target] cfake_cleanup_module(), stack entries: 4
[cr_target] stack entry #0: [<c0276685>] sys_delete_module+0x145/0x220
[cr_target] stack entry #1: [<c0203190>] sysenter_do_call+0x12/0x22
[cr_target] stack entry #2: [<ffffe430>] 0xffffe430
[cr_target] stack entry #3: [<ffffffff>] 0xffffffff 
----------------------------

At most TEST_STACK_ENTRIES stack entries are obtained each time. You can 
experiment with this value - see 
	#define TEST_STACK_ENTRIES <...>
in cfake.c.

TEST_STACK_ENTRIES must be positive and must not exceed KEDR_MAX_FRAMES 
(16 by default).
========================================================================
