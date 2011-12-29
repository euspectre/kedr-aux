module_name = payload

KBUILD_DIR = /lib/modules/`uname -r`/build
PWD=`pwd`

all: $(module_name).ko

$(module_name).ko: payload.c
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) clean

PHONY: all clean