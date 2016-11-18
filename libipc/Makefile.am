AUTOMAKE_OPTIONS=subdir-objects

SUBDIRS=tests

noinst_LIBRARIES=libarmadito-ipc.a

libarmadito_ipc_a_SOURCES= \
hash.c \
hash.h \
common.h \
serialfuns.c \
deserialfuns.c 

AM_CPPFLAGS= -I$(top_srcdir)/libipc/include -I$(top_srcdir)/libmodule/include -I$(top_srcdir) @LIBJANSSON_CFLAGS@
