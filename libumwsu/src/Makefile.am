lib_LTLIBRARIES=libumwsu.la

libumwsu_la_SOURCES=module.c modulep.h dir.c dir.h umwsu.c

AM_CPPFLAGS= -DLIBUMWSU_MODULES_PATH='"$(pkglibdir)/modules"'
libumwsu_la_CFLAGS= -I$(top_srcdir)/include
libumwsu_la_LIBADD= 

libumwsu_la_CFLAGS+= @GMODULE2_CFLAGS@
libumwsu_la_LIBADD+= @GMODULE2_LIBS@

