
BIN=main
OBJS=main.o src/dbus.o src/fanotify.o

CFLAGS=$(shell pkg-config --cflags gio-2.0 gio-unix-2.0) -Iincludes/ -g
LDLIBS=$(shell pkg-config --libs gio-2.0 gio-unix-2.0)

all: $(BIN)

clean:
	-rm -f $(OBJS) $(BIN)

$(BIN): $(OBJS)

all-1:
	gcc -g -o rtime -Wall -Wextra -Iincludes \
	-I/usr/include/dbus-1.0 \
    -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include \
    -I/usr/include/glib-2.0 \
    -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ \
    -ldbus-1 \
    -I/usr/include/gio-unix-2.0 \
    -ldbus-glib-1 src/* main.c

