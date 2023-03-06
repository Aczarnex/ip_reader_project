all:
	gcc ip_reader.c -o reader
clean:
	-rm -f reader
