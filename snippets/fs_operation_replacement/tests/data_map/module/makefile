module_name := test_module
src_dir := ../../..

all: $(module_name).ko

KBUILD_DIR = /lib/modules/`uname -r`/build

$(module_name).ko : module.c data_map.c
	make -C $(KBUILD_DIR) M=`pwd` modules

data_map.c: $(src_dir)/data_map.c
	cp -p $(src_dir)/data_map.c data_map.c

clean:
	make -C $(KBUILD_DIR) M=`pwd` clean
	rm -f data_map.c

.PHONY: all clean
