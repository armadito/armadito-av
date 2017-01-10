SUBDIRS= jrpc

AUTOMAKE_OPTIONS=subdir-objects

noinst_LIBRARIES=librpc.a

librpc_a_SOURCES= \
rpcdefs.h \
rpctypes.c \
rpctypes.h

librpc_a_CFLAGS=-I$(top_srcdir) -I$(top_srcdir)/libmodule/include -I$(top_srcdir)/libcore/include -I$(top_srcdir)/librpc/jrpc/include @LIBJANSSON_CFLAGS@
