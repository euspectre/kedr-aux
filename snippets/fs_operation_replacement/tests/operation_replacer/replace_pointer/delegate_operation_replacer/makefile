module_name=delegate_operation_replacer
replacer_src_dir=./../../../..

all: $(module_name).ko

KBUILD_DIR=/lib/modules/`uname -r`/build

$(module_name).ko: delegate_operation_replacer_module.c operation_replacer.c data_map.c
	make -C $(KBUILD_DIR) M=`pwd` modules

operation_replacer.c: $(replacer_src_dir)/operation_replacer.c
	cp -p $(replacer_src_dir)/operation_replacer.c .

data_map.c: $(replacer_src_dir)/data_map.c
	cp -p $(replacer_src_dir)/data_map.c .

clean:
	make -C $(KBUILD_DIR) M=`pwd` clean
	rm -f operation_replacer.c data_map.c

.PHONY: all clean