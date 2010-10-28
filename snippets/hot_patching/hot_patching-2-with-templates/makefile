include common_makefile
# Because next include file define its own rules, need to define 'all' rule
all: $(module_name).ko

include generate_sources

payloads_all_data := payloads_all.data
# Derived directories of the KEDR (should be corrected, if needed)
kedr_generator := $(kedr_build_dir)/kedr_gen.install/kedr_gen
templates_dir := ./templates



generate_sources:  $(payloads_all_data)
	$(kedr_generator) $(templates_dir)/generate_sources $< > $@

KBUILD_DIR := /lib/modules/`uname -r`/build
PWD := `pwd`

$(module_name).ko: patcher.c payloads_module_functions.c $(payload_c_files) $(payload_h_files)
	make -C $(KBUILD_DIR) M=$(PWD) modules

payloads_module_functions.c: $(payloads_all_data)
	$(kedr_generator) $(templates_dir)/payloads_module_functions.c $< > $@

$(payloads_all_data): $(payload_data_files)
	cat $^ > $@
	printf "$(foreach payload_datafile, $^, payload.datafile = $(payload_datafile)\n)" >> $@

clean:
	make -C $(KBUILD_DIR) M=$(PWD) clean
	rm -f $(payload_c_files) $(payload_h_files)
	rm -f payloads_module_functions.c
	rm -f $(payloads_all_data)
	rm -f generate_sources

.PHONY: all clean