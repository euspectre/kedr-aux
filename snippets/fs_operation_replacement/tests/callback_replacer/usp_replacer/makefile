module_name=usp_replacer

src_dir=../../..

KBUILD_DIR := /lib/modules/`uname -r`/build

all: $(module_name).ko


$(module_name).ko: usp_replacer_module.c callback_replacer.c data_map.c
	make -C $(KBUILD_DIR) M=`pwd` modules

callback_replacer.c: $(src_dir)/callback_replacer.c
	cp -p $(src_dir)/callback_replacer.c .

data_map.c: $(src_dir)/data_map.c
	cp -p $(src_dir)/data_map.c .

clean:
	make -C $(KBUILD_DIR) M=`pwd` clean
	rm -f callback_replacer.c data_map.c

.PHONY: all clean
