module_name=char_counter

process_user_string_symvers := ../process_user_string/Module.symvers

all: $(module_name).ko

KBUILD_DIR=/lib/modules/`uname -r`/build

$(module_name).ko: char_counter.c $(process_user_string_symvers)
	cp -p $(process_user_string_symvers) ./Module.symvers
	make -C $(KBUILD_DIR) M=`pwd` modules

clean:
	make -C $(KBUILD_DIR) M=`pwd` clean

.PHONY: all clean