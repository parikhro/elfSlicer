CC = gcc
CFLAGS = -g -Wall

all: slicer test header progHeader sectHeader

slicer: elfSlicer.c
	$(CC) $(CFLAGS) -o elfSlicer elfSlicer.c -lelf

header: getElfHeader.c
	$(CC) $(CFLAGS) -o getElfHeader getElfHeader.c -lelf -lbsd

progHeader: getElfProgHeader.c
	$(CC) $(CFLAGS) -o getElfProgHeader getElfProgHeader.c -lelf -lbsd

sectHeader: getElfProgHeader.c
	$(CC) $(CFLAGS) -o getElfSectHeader getElfSectHeader.c -lelf -lbsd

test: testOpenFile.c
	$(CC) $(CFLAGS) -o testOpenFile testOpenFile.c -lelf

clean:
	rm -f testOpenFile elfSlicer getElfHeader getElfProgHeader getElfSectHeader
