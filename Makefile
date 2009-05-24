CFLAGS=-Wall
CXXFLAGS=-Wall

all: multicron

OBJECTS=cnproc.o ezxml.o xml.o main.o inotify.o
multicron: $(OBJECTS)
	g++ $(OBJECTS) -o multicron -Wall

clean:
	rm -f multicron $(OBJECTS)
