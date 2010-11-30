MODULE_NAME := kedr_leak_check

# The directory where KEDR is installed
KEDR_INSTALL_DIR ?= /usr/local

# The payload module uses the symbols exported by the KEDR core modules and
# hence, needs appropriate .symvers file. 
KEDR_CORE_SYMVERS := $(KEDR_INSTALL_DIR)/lib/modules/$(shell uname -r)/symvers/kedr_base.symvers

# Some kernels (e.g. 2.6.27) seem not to support KBUILD_EXTRA_SYMBOLS, so 
# we have to resort to the older approach and name the .symvers file from 
# KEDR core as 'Module.symvers' too.
KEDR_SYMVERS_NAME := Module.symvers

# It seems that not each variant of the kernel provides 
# kmem_cache_alloc_notrace() function. 
# KEDR itself has a nice configuration system  to handle such situations 
# but let us resort to a simpler approach for now.
KMCA_NOTRACE := $(shell (grep -q '\bkmem_cache_alloc_notrace' /proc/kallsyms; echo $$?))
ifeq ($(KMCA_NOTRACE),0)
EXTRA_DEFINES := -DKEDR_HAVE_KMCA_NOTRACE
else
EXTRA_DEFINES := 
endif

ifeq ($(KERNELRELEASE),)
# 'Out-of-kernel' part
# It is used in the first place when the user executes 'make [target]'.
# Eventually, it will invoke the kernel build system and that, in turn, will
# execute the 'KBuild' part of this Makefile
# (see "$(MAKE) -C $(KERNELDIR) M=$(PWD)" stuff).

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules: $(KEDR_CORE_SYMVERS)
	cp "$(KEDR_CORE_SYMVERS)" $(KEDR_SYMVERS_NAME)
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

.PHONY: modules modules_install clean

else
# KBuild part. 
# It is used by the kernel build system to actually build the module.

ccflags-y := -I$(src) -I$(KEDR_INSTALL_DIR)/include $(EXTRA_DEFINES)

obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-y := \
	payload.o \
	kedr_stack_trace.o

endif
