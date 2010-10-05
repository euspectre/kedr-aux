module_name=dynamic_control_files_example

KBUILD_DIR=/lib/modules/`uname -r`/build
PWD=`pwd`

all: ${module_name}.ko

${module_name}.ko: module.c
	make -C ${KBUILD_DIR} M=${PWD} modules

clean:
	make -C ${KBUILD_DIR} M=${PWD} clean

.PHONY: all clean
