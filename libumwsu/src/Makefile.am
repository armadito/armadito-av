lib_LTLIBRARIES=libumwsu.la

libumwsu_la_SOURCES= \
alert.c \
alert.h \
dir.c \
dir.h \
module.c \
modulep.h \
report.c \
reportp.h \
scan.c \
status.c \
statusp.h \
umwsu.c \
umwsup.h \
unixsock.c \
unixsock.h 

#protocol.c 
#protocol.h 

AM_CPPFLAGS= -DLIBUMWSU_MODULES_PATH='"$(pkglibdir)/modules"'
libumwsu_la_CFLAGS= -I$(top_srcdir)/include
libumwsu_la_LIBADD= 

libumwsu_la_CFLAGS+= @GMODULE2_CFLAGS@
libumwsu_la_LIBADD+= @GMODULE2_LIBS@

libumwsu_la_CFLAGS+= @LIBXML2_CFLAGS@
libumwsu_la_LIBADD+= @LIBXML2_LIBS@

libumwsu_la_CFLAGS+= @LIBCURL_CFLAGS@
libumwsu_la_LIBADD+= @LIBCURL_LIBS@
