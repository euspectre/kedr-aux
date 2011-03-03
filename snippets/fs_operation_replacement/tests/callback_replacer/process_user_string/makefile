module_name=process_user_string

usp_replacer_symvers := ../usp_replacer/Module.symvers

all: $(module_name).ko

KBUILD_DIR=/lib/modules/`uname -r`/build

$(module_name).ko: process_user_string_module.c $(usp_replacer_symvers)
	cp -p $(usp_replacer_symvers) ./Module.symvers
	make -C $(KBUILD_DIR) M=`pwd` modules

clean:
	make -C $(KBUILD_DIR) M=`pwd` clean

.PHONY: all clean