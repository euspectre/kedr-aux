A user-space application to demonstrate & debug instrumentation on.
The application imitates several constructs that can be encountered in the 
kernel modules.

[Code]

hello.c - the source code of the application itself.
stubs.c - implementation of the stubs that imitate imported functions used 
	  in the kernel code.

my_funcs.c - the handlers.

[Build]

gcc -g -O2 -c -o my_funcs.o my_funcs.c
gcc -g -O2 -c -o stubs.o stubs.c
gcc -g -O2 -o hello -fplugin=<path_to_kmodule_test_plugin> \
	hello.c stubs.o my_funcs.o

To debug the instrumentation, -fdump-tree-ssa-raw and -fdump-tree-einline-raw 
can be added to to dump the IR. "SSA" pass is before the instrumentation, 
"Einline" pass comes after it.

[Run]

Different code paths can be executed:

./hello 1 2 3
./hello
./hello 1 2

See the output and look at the corresponding places in hello.c.
