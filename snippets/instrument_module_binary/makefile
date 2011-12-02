all: instrument_module

instrument_module: instrument_module.c
	$(CC) -o $@ $^ -lelf

clean:
	rm -f instrument_module

.PHONY: all clean