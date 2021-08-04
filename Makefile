CC=gcc
CFLAGS=-g -Wall `pkg-config --cflags gtk+-3.0`
LDFLAGS=-lxcb `pkg-config --libs gtk+-3.0` -lm

.PHONY: all

all: unitconv

unitconv: unitconv.c
	$(CC) -Wall -o unitconv unitconv.c $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f unitconv *.o
