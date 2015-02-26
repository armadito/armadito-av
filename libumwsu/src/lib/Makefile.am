lib_LTLIBRARIES=libumwsu.la

libumwsu_la_SOURCES= \
action.c \
alert.c \
alert.h \
conf.c \
conf.h \
dir.c \
dir.h \
module.c \
modulep.h \
protocol.c \
protocol.h \
quarantine.c \
quarantine.h \
report.c \
reportp.h \
scan.c \
status.c \
statusp.h \
umwsu.c \
umwsup.h \
unixsock.c \
unixsock.h \
watch.c \
watchp.h


AM_CPPFLAGS= -DLIBUMWSU_MODULES_PATH='"$(pkglibdir)/modules"' -DLIBUMWSU_CONF_DIR='"$(sysconfdir)/umwsu"' -DLIBUMWSU_ALERT_DIR='"$(localstatedir)/spool/uhuru"' 
libumwsu_la_CFLAGS= -I$(top_srcdir)/include
libumwsu_la_LIBADD= 

libumwsu_la_CFLAGS+= @GMODULE2_CFLAGS@
libumwsu_la_LIBADD+= @GMODULE2_LIBS@

libumwsu_la_CFLAGS+= @LIBXML2_CFLAGS@
libumwsu_la_LIBADD+= @LIBXML2_LIBS@

#libumwsu_la_CFLAGS+= @LIBCURL_CFLAGS@
#libumwsu_la_LIBADD+= @LIBCURL_LIBS@

install-exec-hook:
	-mkdir -p $(DESTDIR)$(localstatedir)/spool/uhuru
	-chmod 0777 $(DESTDIR)$(localstatedir)/spool/uhuru

