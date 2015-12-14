
BIN=main
OBJS=main.o fanotify.o

CFLAGS=$(shell pkg-config --cflags gio-2.0 gio-unix-2.0) -g
LDLIBS=$(shell pkg-config --libs gio-2.0 gio-unix-2.0)

all: $(BIN)

clean:
	-rm -f $(OBJS) $(BIN)

$(BIN): $(OBJS)
