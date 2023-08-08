CC = gcc
CFLAGS = -g -Wall -lelf #-fsanitize=address,undefined

all: slicer test

slicer: elfSlicer.c
	$(CC) $(CFLAGS) -o elfSlicer elfSlicer.c

test: testOpenFile.c
	$(CC) $(CFLAGS) -o testOpenFile testOpenFile.c

clean:
	rm -f testOpenFile elfSlicer
