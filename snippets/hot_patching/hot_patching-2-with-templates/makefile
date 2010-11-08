include common_makefile

all: $(module_name).ko

payloads_all_data := payloads_all.data
# Derived directories of the KEDR (should be corrected, if needed)
kedr_generator := $(kedr_build_dir)/kedr_gen.install/kedr_gen
templates_dir := ./templates

KBUILD_DIR := /lib/modules/`uname -r`/build
PWD := `pwd`

$(module_name).ko: patcher.c payload.c trace_payload.h copy_arch_tree
	make -C $(KBUILD_DIR) M=$(PWD) modules

payload.c: $(payloads_all_data)
	$(kedr_generator) $(templates_dir)/payload.c $< > $@

trace_payload.h: $(payloads_all_data)
	$(kedr_generator) $(templates_dir)/trace_payload.h $< > $@

$(payloads_all_data): $(payload_data_files)
	cat $^ > $@

copy_arch_tree:
	cp -pr $(kedr_source_dir)/controller/arch ./

clean:
	make -C $(KBUILD_DIR) M=$(PWD) clean
	rm -f payload.c trace_payload.h
	rm -f $(payloads_all_data)
	rm -f generate_sources

.PHONY: all clean copy_arch_tree