CFLAGS=-Wall -fPIC -g `pcre-config --cflags-posix`
CXXFLAGS=-Wall -fPIC -g `pcre-config --cflags-posix`

all: multicron

UEVENT=uevent.o uevent/power.o
OBJECTS=cnproc.o ezxml.o xml.o main.o inotify.o commands.o regexp.o date.o $(UEVENT)

multicron: $(OBJECTS)
	g++ $(OBJECTS) -o multicron -Wall -g `pcre-config --libs-posix`

clean:
	rm -f multicron $(OBJECTS)
