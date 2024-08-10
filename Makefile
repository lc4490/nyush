CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all
all: nyush

nyush: nyush.o shell.o

nyush.o: nyush.c shell.h

shell.o: shell.c shell.h

.PHONY: clean
clean:
	rm -f *.o nyush
