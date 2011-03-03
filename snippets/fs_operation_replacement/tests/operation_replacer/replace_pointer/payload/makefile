module_name=payload

delegate_operation_replacer_symvers := ../delegate_operation_replacer/Module.symvers

all: $(module_name).ko

KBUILD_DIR=/lib/modules/`uname -r`/build

$(module_name).ko: payload_common.c $(delegate_operation_replacer_symvers)
	cp -p $(delegate_operation_replacer_symvers) ./Module.symvers
	make -C $(KBUILD_DIR) M=`pwd` modules

clean:
	make -C $(KBUILD_DIR) M=`pwd` clean

.PHONY: all clean