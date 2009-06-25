CFLAGS=-Wall -fPIC -g `pcre-config --cflags-posix`
CXXFLAGS=-Wall -fPIC -g `pcre-config --cflags-posix`

all: multicron

UEVENT=uevent.o uevent/power.o uevent/usb.o uevent/input.o
#OBJECTS=cnproc.o ezxml.o cfg.o main.o inotify.o input.o commands.o regexp.o date.o $(UEVENT)
OBJECTS=ezxml.o cfg.o main.o commands.o regexp.o

MODULES=date.so inotify.so cnproc.so uevent.so input.so

multicron: $(OBJECTS)
	g++ $(OBJECTS) -o multicron -Wall -g `pcre-config --libs-posix` -ldl -rdynamic

uevent.so: $(UEVENT)
	gcc $(UEVENT) -shared -o $@  -rdynamic -g

%.so: %.cpp
	gcc $< -shared -o $@  -rdynamic -g

all: multicron $(MODULES)

clean:
	rm -f multicron $(OBJECTS)
