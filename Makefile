CFLAGS=-g -Wall -Wextra -std=c11 -pedantic
CC=gcc

lib: lib.c
	$(CC) $(CFLAGS) -o lib lib.c

clean:
	rm -f lib
