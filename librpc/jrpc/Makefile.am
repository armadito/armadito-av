AUTOMAKE_OPTIONS=subdir-objects

# build current directory before tests
#see: https://www.gnu.org/software/automake/manual/html_node/Subdirectories.html
SUBDIRS=. tests

noinst_LIBRARIES=libjrpc.a

libjrpc_a_SOURCES= \
buffer.c \
buffer.h \
connection.c \
connection.h \
hash.c \
hash.h \
jrpc.c \
mapper.c \
marshall.c 

# for debug, add:
# -DJRPC_DEBUG 
AM_CPPFLAGS= -DJRPC_DEBUG $(PTHREAD_CFLAGS) -I$(srcdir)/include -I$(srcdir) -I$(top_srcdir)/libmodule/include -I$(top_srcdir) @LIBJANSSON_CFLAGS@ -save-temps 

noinst_HEADERS= \
include/libjrpc/jrpc.h \
include/libjrpc/marshall.h
