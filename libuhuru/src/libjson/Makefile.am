AUTOMAKE_OPTIONS=subdir-objects

lib_LTLIBRARIES=libuhuru_json.la

libuhuru_json_la_SOURCES= \
uhurujson.c \
uhurujson.h 

AM_CPPFLAGS=-DG_LOG_DOMAIN='"uhuru"'
libuhuru_json_la_CFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/include/config -I$(top_srcdir)/src
libuhuru_json_la_LIBADD= 

libuhuru_json_la_CFLAGS+= @LIBJSONC_CFLAGS@
libuhuru_json_la_LIBADD+= @LIBJSONC_LIBS@
