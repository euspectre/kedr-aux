module_name = trace_server

KBUILD_DIR = /lib/modules/`uname -r`/build
PWD=`pwd`

all: $(module_name).ko

$(module_name).ko: trace_server.c
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) clean

PHONY: all clean