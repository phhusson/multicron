CFLAGS=-Wall -fPIC
CXXFLAGS=-Wall -fPIC

all: multicron

OBJECTS=cnproc.o ezxml.o xml.o main.o inotify.o commands.o regexp.o date.o
multicron: $(OBJECTS)
	g++ $(OBJECTS) -o multicron -Wall -lpcreposix

clean:
	rm -f multicron $(OBJECTS)
