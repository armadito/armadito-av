TARGET=nautilus_ext.so
OBJECTS=plugin.o

LIBDIR=/usr/lib
WFLAGS=-Wall -pedantic -Wmissing-prototypes
CFLAGS= $(WFLAGS) -fPIC -g \
	$(shell pkg-config --cflags libnautilus-extension)

LDFLAGS=-shared \
	$(shell pkg-config --libs libnautilus-extension)

all: $(TARGET)

$(TARGET): $(OBJECTS)
		gcc $(LDFLAGS) $(OBJECTS) -o $(TARGET)

install: all
	#mkdir -p $(LIBDIR)/nautilus/extensions-2.0
	cp $(TARGET) $(LIBDIR)/nautilus/extensions-3.0/

plugin.o: plugin.c
	gcc -c $(CFLAGS) plugin.c -o plugin.o

clean:
	rm -f $(OBJECTS) $(TARGET)
