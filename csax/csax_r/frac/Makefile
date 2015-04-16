CXX ?= g++
CFLAGS = -Wall -Wconversion -O3 -fPIC
SHVER = 2
SRC = src

all: frac

lib: svm.o
	$(CXX) -shared -dynamiclib svm.o -o libsvm.so.$(SHVER)

frac: ${SRC}/main.cpp svm.o frac.o
	$(CXX) $(CFLAGS) ${SRC}/main.cpp svm.o frac.o -o frac -lm

svm.o: ${SRC}/svm.cpp ${SRC}/svm.h
	$(CXX) $(CFLAGS) -c ${SRC}/svm.cpp

frac.o: ${SRC}/frac.h ${SRC}/frac.c
	$(CXX) $(CFLAGS) -c ${SRC}/frac.c 

clean:
	rm -f *~ svm.o frac.o libsvm.so.$(SHVER)

distclean: clean
	rm -f frac

build: distclean all
