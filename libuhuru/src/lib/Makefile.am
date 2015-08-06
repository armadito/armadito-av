lib_LTLIBRARIES=libuhuru.la

libuhuru_la_SOURCES= \
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
remote.c \
remote.h \
report.c \
reportp.h \
scan.c \
status.c \
statusp.h \
uhuru.c \
uhurup.h \
unixsock.c \
unixsock.h 


AM_CPPFLAGS= -DLIBUHURU_MODULES_PATH='"$(pkglibdir)/modules"' -DLIBUHURU_CONF_DIR='"$(sysconfdir)/uhuru"' -DLIBUHURU_ALERT_DIR='"$(localstatedir)/spool/uhuru"' -DG_LOG_DOMAIN='"libuhuru"'
libuhuru_la_CFLAGS= -I$(top_srcdir)/include
libuhuru_la_LIBADD= 

libuhuru_la_CFLAGS+= @GMODULE2_CFLAGS@
libuhuru_la_LIBADD+= @GMODULE2_LIBS@

libuhuru_la_CFLAGS+= @LIBXML2_CFLAGS@
libuhuru_la_LIBADD+= @LIBXML2_LIBS@

#libuhuru_la_CFLAGS+= @LIBCURL_CFLAGS@
#libuhuru_la_LIBADD+= @LIBCURL_LIBS@

install-exec-hook:
	-mkdir -p $(DESTDIR)$(localstatedir)/spool/uhuru
	-chmod 0777 $(DESTDIR)$(localstatedir)/spool/uhuru

