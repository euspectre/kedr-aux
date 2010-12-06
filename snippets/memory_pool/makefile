# Name of the output call monitor payload module
module_name := payload_memory_pool

kedr_base_symbols := /home/andrew/cmake-install-dir/lib/modules/2.6.32-21-generic/symvers/kedr_base.symvers

KBUILD_DIR=/lib/modules/`uname -r`/build
PWD=`pwd`

all: $(module_name).ko

$(module_name).ko: payload.c memory_pool.c $(kedr_base_symbols)
	cat $(kedr_base_symbols) > ./Module.symvers
	make -C $(KBUILD_DIR) M=$(PWD) modules

clean:
	make -C $(KBUILD_DIR) M=$(PWD) clean

.PHONY: all clean
