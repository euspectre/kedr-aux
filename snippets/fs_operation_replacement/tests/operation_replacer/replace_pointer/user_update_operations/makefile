module_name=delegate_impl_update_operations

delegate_base_symvers := ../base_update_replacement/Module.symvers

all: $(module_name).ko

KBUILD_DIR=/lib/modules/`uname -r`/build

$(module_name).ko: delegate_impl.c $(delegate_base_symvers)
	cp -p $(delegate_base_symvers) ./Module.symvers
	make -C $(KBUILD_DIR) M=`pwd` modules

delegate_impl.c: ../user/delegate_impl.c
	cp -p ../user/delegate_impl.c .

clean:
	make -C $(KBUILD_DIR) M=`pwd` clean
	rm -f delegate_impl.c

.PHONY: all clean