AUTOMAKE_OPTIONS=subdir-objects

# build current directory before tests
#see: https://www.gnu.org/software/automake/manual/html_node/Subdirectories.html
SUBDIRS=. tests

noinst_LIBRARIES=libarmadito-rpc.a

libarmadito_rpc_a_SOURCES= \
buffer.c \
buffer.h \
common.h \
connection.c \
connection.h \
hash.c \
hash.h \
mapper.c \
mapper.h \
marshall.h \
marshallfuns.c \
rpc.c \
unmarshallfuns.c

AM_CPPFLAGS= -I$(top_srcdir)/librpc/include -I$(top_srcdir)/libmodule/include -I$(top_srcdir) -I$(top_srcdir)/librpc @LIBJANSSON_CFLAGS@
