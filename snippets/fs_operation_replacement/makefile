module_name=fs_replacer
all: $(module_name).ko

sources:= data_map.c operation_replacer.c fs_operation_replacement.c callback_replacer.c payload.c
headers:= data_map.h operation_replacer.h fs_operation_replacement.h callback_replacer.h
pwd=`pwd`
kbuild_dir=/lib/modules/`uname -r`/build

kedr_install_dir=/home/andrew/kedr-builds/common/install
kedr_base_symbols=$(kedr_install_dir)/lib/modules/`uname -r`/symvers/kedr_base.symvers

$(module_name).ko: $(sources) $(header)
	cat $(kedr_base_symbols) > Module.symvers
	$(MAKE) -C $(kbuild_dir) M=$(pwd) modules

clean:
	$(MAKE) -C $(kbuild_dir) M=$(pwd) clean

.PHONY: all clean
