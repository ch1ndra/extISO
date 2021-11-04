CC=gcc
CFLAGS=-Wall -Wextra -O2

all: iso9660lib extiso

iso9660lib: 
	$(CC) $(CFLAGS) -c iso9660.c -o iso9660.o
	
extiso: iso9660lib
	$(CC) $(CFLAGS) extiso.c iso9660.o -o extiso

clean:
	rm *.o
