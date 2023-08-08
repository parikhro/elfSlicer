CC = gcc
CFLAGS = -g -Wall

all: slicer test header

slicer: elfSlicer.c
	$(CC) $(CFLAGS) -o elfSlicer elfSlicer.c -lelf

header: getElfHeader.c
	$(CC) $(CFLAGS) -o getElfHeader getElfHeader.c -lelf -lbsd

test: testOpenFile.c
	$(CC) $(CFLAGS) -o testOpenFile testOpenFile.c -lelf

clean:
	rm -f testOpenFile elfSlicer getElfHeader
