# Name of the output indicator module
module_name := indicator_fsim_common_ca

KEDR_INSTALL_PREFIX=/usr/local
KERNEL_VERSION=$(shell uname -r)

indicator_data_file := indicator.data

indicator_c_file := indicator.c
indicator_internal_data_file := indicator_internal.data

additional_sources = calculator.c control_file.c

kedr_gen_templates_dir := $(KEDR_INSTALL_PREFIX)/share/kedr/templates
kedr_gen_tool := $(KEDR_INSTALL_PREFIX)/lib/kedr/kedr_gen

kedr_symbols := $(KEDR_INSTALL_PREFIX)/lib/modules/$(KERNEL_VERSION)/symvers/kedr.symvers
fault_simulation_symbols := $(KEDR_INSTALL_PREFIX)/lib/modules/$(KERNEL_VERSION)/symvers/kedr_fault_simulation.symvers

KBUILD_DIR=/lib/modules/$(KERNEL_VERSION)/build
PWD=`pwd`

all: $(module_name).ko

$(module_name).ko: $(indicator_c_file) $(additional_sources) $(kedr_symbols) $(fault_simulation_symbols)
	cat "$(kedr_symbols)" "$(fault_simulation_symbols)" > ./Module.symvers
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) modules

$(indicator_c_file): $(indicator_internal_data_file)
	$(kedr_gen_tool) $(kedr_gen_templates_dir)/fault_indicator.c/ $^ > $@

indicator_data_file_with_caller_address := indicator_with_caller_address.data

$(indicator_internal_data_file): $(indicator_data_file_with_caller_address)
		$(kedr_gen_tool) templates/fault_indicator.data/ $^ > $@

$(indicator_data_file_with_caller_address): $(indicator_data_file)
		$(kedr_gen_tool) $(kedr_gen_templates_dir)/fault_indicator_add_caller_address.data/ $^ > $@

clean:
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) clean
	rm -f $(indicator_c_file)
	rm -f $(indicator_internal_data_file)
	rm -f $(indicator_data_file_with_caller_address)

.PHONY: all clean
