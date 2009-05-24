CFLAGS=-Wall
CXXFLAGS=-Wall

all: multicron

OBJECTS=cnproc.o ezxml.o xml.o main.o inotify.o commands.o regexp.o
multicron: $(OBJECTS)
	g++ $(OBJECTS) -o multicron -Wall -lpcreposix

clean:
	rm -f multicron $(OBJECTS)
