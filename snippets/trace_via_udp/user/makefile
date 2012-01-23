all: trace_reader

CFLAGS := -I../

trace_reader: trace_reader.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f ./trace_reader

.PHONY: all clean