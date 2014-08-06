modulesdir=@LIBUMWSU_MODDIR@
modules_LTLIBRARIES=clamav.la

clamav_la_SOURCES=clamav.c

clamav_la_LDFLAGS= -module -avoid-version
clamav_la_CFLAGS=
clamav_la_LIBADD=
clamav_la_CFLAGS+= @LIBUMWSU_CFLAGS@
clamav_la_LIBADD+= @LIBUMWSU_LIBS@
clamav_la_CFLAGS+= @LIBCLAMAV_CFLAGS@
clamav_la_LIBADD+= @LIBCLAMAV_LIBS@

