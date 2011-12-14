all: instrument_module

instrument_module: instrument_module.c
	$(CC) -g -Wall -o $@ $^ -lelf

clean:
	rm -f instrument_module

.PHONY: all clean