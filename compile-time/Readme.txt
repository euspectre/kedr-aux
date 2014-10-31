This plugin demonstrates compile-based instrumentation of memory accesses
and function calls that can be used for kernel-mode components.

Note. It is by no means a complete analysis tool but rather a prototype that
allows to check how things work.

Two examples are provided: a kernel module (samples/sample_target) and a 
user-space app (samples/hello). The latter can be used to simplify debugging
of the instrumentation.

Processed:
- memory reads and writes (most of this code is from gcc/tsan.c as of GCC 
4.9);
- function entries and exits - can also be used in the future to process 
callbacks like netdev_ops, file_operations, etc., and report the related
locking and signal/wait events, and so on;
- function calls: pre- and post-handlers can be set; also, a call (e.g., 
__kmalloc() in the examples) can be replaced with a call to a user-supplied 
function to allow fault simulation, etc.

All this should allow to implement collection of data needed to detect 
races as well as checking for memory leaks and fault simulation that KEDR 
0.x already does.

At the moment, the instrumentation happens before GCC performs inlining of
functions.

Note. At the entry to each function, a memory block called "local storage"  
is allocated. Its address is passed to each event handler. The storage can 
be used to pass the arguments of the functions from pre- to post- handlers, 
to store thread IDs, etc. At the exit from the function, the local storage 
is destroyed.

[Prerequisites]

1. GCC 4.8 or newer.
2. Headers and other stuff for development of GCC plugins (see something 
like gcc-plugin-devel or gcc-plugin-dev in your package manager).
3. g++
4. Cmake 2.8 or newer (3.0 or newer is recommended).
5. Development files for the kernel (to build the example).

[Build and Usage]

1. Create a top build directory, configure and build the plugin:
	mkdir build
	cd build
	cmake <path_to_source_dir_kmodule-test>
	make

src/kmodule-test.so plugin will be built.

2. See samples/hello/Readme.txt for the instructions how to use that plugin 
when building the user-space application.

3. The sources of the kernel-mode example (sample_target) are now in the 
build tree, in samples/sample_target. Use
	make -f Makefile.mk
to build it.

4. Load the module and do something with the character devices it maintains
(as root):

# insmod kedr_sample_target.ko
# echo Something > /dev/cfake0
# dd if=/dev/cfake0 bs=30 count=1
# dd if=/dev/cfake1 bs=30 count=1
# rmmod kedr_sample_target

5. The output is in the system log, which is OK for this example. In a real 
system, it will be a more complex task to properly output and save the data 
(or process them on-the-fly).

Part of the log (shortened):
[ 3874.056161] [DBG] Function entry, func = cfake_open [kedr_sample_target]
[ 3874.056238] [DBG] Function entry, func = imajor [kedr_sample_target]
[ 3874.056283] [DBG] TID=d4896b80: memory read at cfake_open+0x63/0x280 [kedr_sample_target]: accessed 4 byte(s) starting from da5377d4.
[ 3874.056378] [DBG] Function exit, func = imajor [kedr_sample_target]
[ 3874.056399] [DBG] Function entry, func = iminor [kedr_sample_target]
[ 3874.056444] [DBG] TID=d4896b80: memory read at cfake_open+0x9d/0x280 [kedr_sample_target]: accessed 4 byte(s) starting from da5377d4.
[ 3874.056460] [DBG] Function exit, func = iminor [kedr_sample_target], my_struct is at f37e6240
[ 3874.056481] [DBG] TID=d4896b80: memory read at cfake_open+0xbc/0x280 [kedr_sample_target]: accessed 4 byte(s) starting from f47cb208.
[ 3874.056502] [DBG] TID=d4896b80: memory read at cfake_open+0xd7/0x280 [kedr_sample_target]: accessed 4 byte(s) starting from f47cb074.
[ 3874.056522] [DBG] TID=d4896b80: memory read at cfake_open+0xf2/0x280 [kedr_sample_target]: accessed 4 byte(s) starting from f47cb204.
[ 3874.056540] [DBG] TID=d4896b80: memory write at cfake_open+0x10a/0x280 [kedr_sample_target]: accessed 4 byte(s) starting from d499c38c.
[ 3874.056559] [DBG] TID=d4896b80: memory read at cfake_open+0x120/0x280 [kedr_sample_target]: accessed 4 byte(s) starting from da5378e0.
[ 3874.056578] [DBG] TID=d4896b80: memory read at cfake_open+0x138/0x280 [kedr_sample_target]: accessed 4 byte(s) starting from e5b8b480.
[ 3874.056598] [DBG] TID=d4896b80: memory read at cfake_open+0x162/0x280 [kedr_sample_target]: accessed 4 byte(s) starting from e5b8b484.
[ 3874.056608] [DBG] pre handler: kmalloc(4000, d0)
[ 3874.056626] [DBG] Function entry, func = kzalloc [kedr_sample_target]
[ 3874.056680] [DBG] pre handler: kmalloc(4000, 80d0)
[ 3874.056702] [DBG] Function entry, func = kmalloc [kedr_sample_target]
[ 3874.056760] [DBG] Replacement for __kmalloc(4000, 80d0) - before the call, ID = d4896b80.
[ 3874.056774] [DBG] Replacement for __kmalloc(4000, 80d0) - after the call (ret = f36e8000).
[ 3874.056792] [DBG] Function exit, func = kmalloc [kedr_sample_target]
[ 3874.056804] [DBG] post handler: kmalloc(4000, 80d0) = f36e8000
[ 3874.056821] [DBG] Function exit, func = kzalloc [kedr_sample_target]
[ 3874.056834] [DBG] post handler: kmalloc(4000, d0) = f36e8000
[ 3874.056855] [DBG] TID=d4896b80: memory write at cfake_open+0x213/0x280 [kedr_sample_target]: accessed 4 byte(s) starting from e5b8b480.
[ 3874.056876] [DBG] TID=d4896b80: memory read at cfake_open+0x21e/0x280 [kedr_sample_target]: accessed 4 byte(s) starting from e5b8b480.
[ 3874.056893] [DBG] Function exit, func = cfake_open [kedr_sample_target]

See cfake_open() in cfake.c for the source code of the function.

[Systems]

Tested on several systems, including, but not limited to:
- ROSA Fresh R4 i586 and x86_64 with GCC 4.9.2
- Fedora 20 x86_64 with GCC 4.8.3
- Ubuntu 14.04 x86_64 with GCC 4.8.2
