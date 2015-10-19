AUTOMAKE_OPTIONS=subdir-objects

lib_LTLIBRARIES=libuhuru_ipc.la

libuhuru_ipc_la_SOURCES= \
ipc.c \
ipc.h

AM_CPPFLAGS= -DG_LOG_DOMAIN='"libuhuru_ipc"'
libuhuru_ipc_la_CFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/include/config -I$(top_srcdir)/src
libuhuru_ipc_la_LIBADD= 

libuhuru_ipc_la_CFLAGS+= @GLIB2_CFLAGS@
libuhuru_ipc_la_LIBADD+= @GLIB2_LIBS@

