module_name = template

KBUILD_DIR = /lib/modules/`uname -r`/build
PWD=`pwd`

PAYLOAD_SYMBOLS = ../payload/Module.symvers

all: $(module_name).ko

$(module_name).ko: template.c $(PAYLOAD_SYMBOLS)
	cat $(PAYLOAD_SYMBOLS) > ./Module.symvers
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) clean

PHONY: all clean