CC=gcc
CFLAGS=-std=c11 -Wall -Wpedantic -g
LDFLAGS=-lreadline

process: process.c
	${CC} ${CFLAGS}	process.c -o process ${LDFLAGS}

clean:
	rm -f *~ *.o process
