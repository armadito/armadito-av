
BIN=fanotify-test
OBJS=main.o monitor.o pollset.o mimetype.o

CFLAGS=-g $(shell pkg-config --cflags glib-2.0)
LDLIBS=$(shell pkg-config --libs glib-2.0) -lmagic

all: $(BIN)

clean:
	-rm -f $(OBJS) $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDLIBS)
