AUTOMAKE_OPTIONS=subdir-objects

# build current directory before tests
#see: https://www.gnu.org/software/automake/manual/html_node/Subdirectories.html
SUBDIRS=. tests

noinst_LIBRARIES=libjrpc.a

libjrpc_a_SOURCES= \
buffer.c \
buffer.h \
common.h \
connection.c \
hash.c \
hash.h \
mapper.c \
marshallfuns.c \
marshallfuns.h \
unmarshallfuns.c \
unmarshallfuns.h

# -DDEBUG because is #defined in top-level config header
AM_CPPFLAGS=-DDEBUG -I$(top_srcdir)/libjrpc/include -I$(top_srcdir)/libmodule/include -I$(top_srcdir) -I$(top_srcdir)/libjrpc @LIBJANSSON_CFLAGS@
