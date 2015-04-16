# Build C programs

CC=gcc
OPTIONS=-O3 -lm -Wall -Wextra 
binaries=lof.l1 lof.l2

all: ${binaries}

lof.l2: oflib.h oflib.c euclidean.c lof.c
	${CC} ${OPTIONS} -o lof.l2 euclidean.c oflib.c lof.c

lof.l1: oflib.h oflib.c manhattan.c lof.c
	${CC} ${OPTIONS} -o lof.l1 manhattan.c oflib.c lof.c

clean:
	/bin/rm -f *.o

distclean:  clean
	/bin/rm -f ${binaries}

build: distclean all
