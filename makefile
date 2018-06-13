CC=gcc
CFLAGS=-std=c11 -Wall -Wpedantic -g
LDFLAGS=-lreadline

hw9: hw9.c
	${CC} ${CFLAGS}	hw9.c -o hw9 ${LDFLAGS}

clean:
	rm -f *~ *.o hw9
