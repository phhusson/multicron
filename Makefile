CFLAGS=-Wall -fPIC -g
CXXFLAGS=-Wall -fPIC -g

all: multicron

OBJECTS=cnproc.o ezxml.o xml.o main.o inotify.o commands.o regexp.o date.o
multicron: $(OBJECTS)
	g++ $(OBJECTS) -o multicron -Wall -lpcreposix -g

clean:
	rm -f multicron $(OBJECTS)
