module_name=payload_special1

delegate_operation_replacer_symvers := ../delegate_operation_replacer/Module.symvers

all: $(module_name).ko

KBUILD_DIR=/lib/modules/`uname -r`/build

$(module_name).ko: payload_common.c $(delegate_operation_replacer_symvers)
	cp -p $(delegate_operation_replacer_symvers) ./Module.symvers
	make -C $(KBUILD_DIR) M=`pwd` modules

payload_common.c: ../payload/payload_common.c
	cp -p ../payload/payload_common.c .

clean:
	make -C $(KBUILD_DIR) M=`pwd` clean
	rm -f payload_common.c

.PHONY: all clean
