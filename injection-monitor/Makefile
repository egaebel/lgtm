CFLAGS = -g -O3 -Wall -static -Wno-unknown-pragmas
LDLIBS = -lorcon -lm -lrt
CC = gcc

ALL = packets_from_file

all: $(ALL)

clean:
	rm -f *.o $(ALL)

packets_from_file: packets_from_file.c util.o

util.c: util.h
