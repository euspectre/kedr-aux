all: instrument_module

CFLAGS = -g -I/home/andrew/ELF/install-152/include

ADDITIONAL_SOURCES := binary_instrument_utilities.c binary_code_template.c \
elf_writer.c

instrument_module: instrument_module.c $(ADDITIONAL_SOURCES)
	$(CC) -g -Wall -o $@ $^ -lelf

clean:
	rm -f instrument_module

.PHONY: all clean