AUTOMAKE_OPTIONS=subdir-objects

SUBDIRS=tests

noinst_LIBRARIES=libarmadito-rpc.a

libarmadito_rpc_a_SOURCES= \
buffer.c \
buffer.h \
common.h \
connection.c \
connection.h \
hash.c \
hash.h \
marshallfuns.c \
rpc.c \
unmarshallfuns.c

AM_CPPFLAGS= -I$(top_srcdir)/librpc/include -I$(top_srcdir)/libmodule/include -I$(top_srcdir) @LIBJANSSON_CFLAGS@
