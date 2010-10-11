module_name=static_control_files_example

KBUILD_DIR=/lib/modules/`uname -r`/build
PWD=`pwd`

all: ${module_name}.ko

${module_name}.ko: module.c control_file.c ../control_file.h
	make -C ${KBUILD_DIR} M=${PWD} modules

control_file.c: ../control_file.c
	cp ../control_file.c ./control_file.c

clean:
	make -C ${KBUILD_DIR} M=${PWD} clean
	rm -f control_file.c

.PHONY: all clean
