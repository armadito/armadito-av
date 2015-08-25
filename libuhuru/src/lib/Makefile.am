AUTOMAKE_OPTIONS=subdir-objects

lib_LTLIBRARIES=libuhuru.la

libuhuru_la_SOURCES= \
action.c \
conf.c \
conf.h \
confparser.c \
confparser.h \
dir.c \
dir.h \
module.c \
modulep.h \
protocol.c \
protocol.h \
report.c \
reportp.h \
scan.c \
status.c \
statusp.h \
uhuru.c \
uhurup.h \
unixsock.c \
unixsock.h \
builtin-modules/alert.c \
builtin-modules/alert.h \
builtin-modules/uhurumod.c \
builtin-modules/uhurumod.h \
builtin-modules/quarantine.c \
builtin-modules/quarantine.h \
builtin-modules/remote.c \
builtin-modules/remote.h 

AM_CPPFLAGS= -DLIBUHURU_MODULES_PATH='"$(pkglibdir)/modules"' -DLIBUHURU_CONF_DIR='"$(sysconfdir)/uhuru"' -DG_LOG_DOMAIN='"libuhuru"'
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

