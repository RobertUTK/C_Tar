CC = gcc
INCLUDES = -I/home/jplank/cs360/include
CFLAGS = -Wall -g -O $(INCLUDES)
LIBDIR = /home/jplank/cs360/lib
LIBS = $(LIBDIR)/libfdr.a

EXECUTABLES = tarc tarx

all: $(EXECUTABLES)

tarc: src/tarc.c fileData.o
	$(CC) $(CFLAGS) -o tarc src/tarc.c fileData.o $(LIBS)

tarx: src/tarx.c fileData.o
	$(CC) $(CFLAGS) -o tarx src/tarx.c fileData.o $(LIBS)

fileData.o: src/fileData.c src/fileData.h
	$(CC) $(INCLUDES) -g -Wall -c src/fileData.c

clean:
	rm -f $(EXECUTABLES) *.o
