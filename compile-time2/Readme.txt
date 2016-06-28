This plugin demonstrates compile-based instrumentation of the function
calls that can be used for kernel-mode components.

The plugin inserts the calls to the special handlers before and after the
calls to the functions (__kmalloc, kfree, etc.)

The handlers used in this example are simple stubs (stubs/kedr_stubs.c).

See the comments in the sources for details.
---------------------

Prerequisites:
1. GCC 5.3 or newer
2. Headers and other stuff for development of GCC plugins (see something 
like gcc-plugin-devel or gcc-plugin-dev in your package manager).
3. g++
4. Cmake 3.0 or newer.
5. Development files for the kernel (to build the example).
---------------------

Building the plugin:

cd <build_dir>
cmake <source_dir>
make
---------------------

Building the example:

cd <build_dir>/tests/common_target/
make -f Makefile.mk
---------------------
