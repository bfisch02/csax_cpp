CC=g++
CFLAGS=-c -Wall -Wextra -std=c++11
LDFLAGS=
SOURCES=main.cpp sample.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=CSAX

all: $(SOURCES) $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

