all:
	gcc main.c -o main.o -c -Wall
	gcc ip_reader.c -o ip_reader.o -c -Wall
	gcc -o reader main.o ip_reader.o
clean:
	rm -f reader
