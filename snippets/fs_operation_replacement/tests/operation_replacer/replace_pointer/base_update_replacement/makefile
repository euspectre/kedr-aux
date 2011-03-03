module_name=delegate_base_update_replacement

delegate_operation_replacer_symvers := ../delegate_operation_replacer/Module.symvers

all: $(module_name).ko

KBUILD_DIR=/lib/modules/`uname -r`/build

$(module_name).ko: base_module.c $(delegate_operation_replacer_symvers)
	cp -p $(delegate_operation_replacer_symvers) ./Module.symvers
	make -C $(KBUILD_DIR) M=`pwd` modules

base_module.c: ../base/base_module.c
	cp -p ../base/base_module.c .

clean:
	make -C $(KBUILD_DIR) M=`pwd` clean
	rm -f base_module.c

.PHONY: all clean