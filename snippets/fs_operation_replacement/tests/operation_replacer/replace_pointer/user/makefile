module_name=delegate_impl

delegate_base_symvers := ../base/Module.symvers

all: $(module_name).ko

KBUILD_DIR=/lib/modules/`uname -r`/build

$(module_name).ko: delegate_impl.c $(delegate_base_symvers)
	cp -p $(delegate_base_symvers) ./Module.symvers
	make -C $(KBUILD_DIR) M=`pwd` modules

clean:
	make -C $(KBUILD_DIR) M=`pwd` clean

.PHONY: all clean