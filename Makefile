CFLAGS=-Wall -fPIC -g
CXXFLAGS=-Wall -fPIC -g

all: multicron

UEVENT=uevent.o uevent/power.o
OBJECTS=cnproc.o ezxml.o xml.o main.o inotify.o commands.o regexp.o date.o $(UEVENT)

multicron: $(OBJECTS)
	g++ $(OBJECTS) -o multicron -Wall -lpcreposix -g

clean:
	rm -f multicron $(OBJECTS)
