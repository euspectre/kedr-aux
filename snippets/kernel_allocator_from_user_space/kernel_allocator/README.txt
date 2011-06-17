Allow to call kmalloc "from user space".

After loading of module, command 
    <debugfs>/kernel_allocator/kmalloc <size>
will call kmalloc(<size>), where <size> is interpreted as decimal number.

Upon successfull function call, result of kmalloc should be retrieved by
    cat <debugfs>/kernel_allocator/last_allocated
which print address in hexadecimal form.

On error, writting to "kmalloc" file returns error(ENOMEM) and reading of "last_allocated" will print "NULL".

Reading from "last_allocated" file will clear its content and one cannot allocate another chunk of memory until "last_allocated" file is not read.

