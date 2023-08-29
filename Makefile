CC = gcc
CFLAGS = -g -Wall

all: slicer

slicer: elfSlicer.c
	$(CC) $(CFLAGS) -o elfSlicer elfSlicer.c -lelf

clean:
	rm -f elfSlicer
