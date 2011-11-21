module_name = vma_fault_interception

include config.mk

# KEDR_INSTALL_PREFIX should be set
#KEDR_INSTALL_PREFIX := /home/andrew/kedr-builds/work/install
# KEDR_COI_INSTALL_PREFIX should be set
#KEDR_COI_INSTALL_PREFIX := /home/andrew/kedr-coi-builds/work/install

KEDR_GEN = $(KEDR_INSTALL_PREFIX)/bin/kedr_gen

KBUILD_DIR = /lib/modules/`uname -r`/build

kedr_templates_dir = $(KEDR_INSTALL_PREFIX)/share/kedr/templates
kedr_symbols = $(KEDR_INSTALL_PREFIX)/lib/modules/`uname -r`/symvers/kedr.symvers

kedr_coi_symbols = $(KEDR_COI_INSTALL_PREFIX)/lib/modules/`uname -r`/symvers/kedr_coi.symvers
kedr_coi_templates_dir = $(KEDR_COI_INSTALL_PREFIX)/share/kedr-coi/templates
kedr_coi_examples_dir = $(KEDR_COI_INSTALL_PREFIX)/share/kedr-coi/examples
kedr_coi_interceptors_dir = $(KEDR_COI_INSTALL_PREFIX)/share/kedr-coi/interceptors
interceptors_dir = $(KEDR_COI_INSTALL_PREFIX)/share/kedr-coi/interceptors

interceptors_normal = vma_operations
interceptors_factory = 
interceptors_creation = vma_operations_clone
interceptors_standard = file_operations inode_file_operations file_system_type super_operations inode_operations dentry_operations

all: $(module_name).ko

$(module_name).ko: vma_fault.c \
    fs_interception.c fs_interception.h \
	$(addsuffix _interceptor.c, $(interceptors_normal) $(interceptors_factory) $(interceptors_creation) $(interceptors_standard)) \
	$(addsuffix _interceptor.h, $(interceptors_normal) $(interceptors_factory) $(interceptors_creation)) \
	functions_support.c
	cat $(kedr_symbols) $(kedr_coi_symbols) > ./Module.symvers
	KEDR_INSTALL_PREFIX=$(KEDR_INSTALL_PREFIX) KEDR_COI_INSTALL_PREFIX=$(KEDR_COI_INSTALL_PREFIX) $(MAKE) -C $(KBUILD_DIR) M=`pwd` modules

fs_interception.c fs_interception.h: % : $(kedr_coi_examples_dir)/file_access_counter/%
	cp -p $^ $@

$(addsuffix _interceptor.c, $(interceptors_normal)): %_interceptor.c: %.data
	$(KEDR_GEN) $(kedr_coi_templates_dir)/kedr_coi_interceptor.c/ $^ > $@

$(addsuffix _interceptor.h, $(interceptors_normal)): %_interceptor.h: %.data
	$(KEDR_GEN) $(kedr_coi_templates_dir)/kedr_coi_interceptor.h/ $^ > $@


$(addsuffix _interceptor.c, $(interceptors_factory)): %_interceptor.c: %.data
	$(KEDR_GEN) $(kedr_coi_templates_dir)/kedr_coi_factory_interceptor.c/ $^ > $@

$(addsuffix _interceptor.h, $(interceptors_factory)): %_interceptor.h: %.data
	$(KEDR_GEN) $(kedr_coi_templates_dir)/kedr_coi_factory_interceptor.h/ $^ > $@

$(addsuffix _interceptor.c, $(interceptors_creation)): %_interceptor.c: %.data
	$(KEDR_GEN) $(kedr_coi_templates_dir)/kedr_coi_creation_interceptor.c/ $^ > $@

$(addsuffix _interceptor.h, $(interceptors_creation)): %_interceptor.h: %.data
	$(KEDR_GEN) $(kedr_coi_templates_dir)/kedr_coi_creation_interceptor.h/ $^ > $@

$(addsuffix _interceptor.c, $(interceptors_standard)): %_interceptor.c: $(kedr_coi_interceptors_dir)/%_interceptor.c
	cp -p $^ $@

functions_support.c: functions.data
	$(KEDR_GEN) $(kedr_templates_dir)/functions_support.c/ $^ > $@

clean:
	$(MAKE) -C $(KBUILD_DIR) M=`pwd` clean
	rm -f functions_support.c
	rm -f $(addsuffix _interceptor.c, $(interceptors_normal) $(interceptors_factory) $(interceptors_creation) $(interceptors_standard))
	rm -f $(addsuffix _interceptor.h, $(interceptors_normal) $(interceptors_factory) $(interceptors_creation))
	rm -f fs_interception.c fs_interception.h

distclean: clean
	rm config.mk
	rm vma_fault_interception.conf

.PHONY: all clean