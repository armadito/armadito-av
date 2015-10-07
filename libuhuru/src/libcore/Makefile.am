AUTOMAKE_OPTIONS=subdir-objects

lib_LTLIBRARIES=libuhuru_core.la

libuhuru_core_la_SOURCES= \
action.c \
info.c \
infop.h \
conf.c \
conf.h \
confparser.c \
confparser.h \
ipcman.c \
ipcman.h \
module.c \
modulep.h \
report.c \
reportp.h \
scan.c \
status.c \
statusp.h \
uhuru.c \
uhurup.h \
builtin-modules/alert.c \
builtin-modules/alert.h \
builtin-modules/mimetypemod.c \
builtin-modules/mimetypemod.h \
builtin-modules/quarantine.c \
builtin-modules/quarantine.h \
builtin-modules/remote.c \
builtin-modules/remote.h \
os/dir.h \
os/file.h \
os/io.h \
os/ipc.h \
os/mimetype.h \
os/string.h \
os/linux/dir.c \
os/linux/file.c \
os/linux/ipc.c \
os/linux/mimetype.c 

AM_CPPFLAGS= -DLIBUHURU_MODULES_PATH='"$(pkglibdir)/modules"' -DLIBUHURU_CONF_DIR='"$(sysconfdir)/uhuru"' -DG_LOG_DOMAIN='"libuhuru"'
libuhuru_core_la_CFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/include/config
libuhuru_core_la_LIBADD= 

libuhuru_core_la_CFLAGS+= @GMODULE2_CFLAGS@
libuhuru_core_la_LIBADD+= @GMODULE2_LIBS@

libuhuru_core_la_CFLAGS+= @LIBXML2_CFLAGS@
libuhuru_core_la_LIBADD+= @LIBXML2_LIBS@

#libuhuru_core_la_CFLAGS+= @LIBCURL_CFLAGS@
#libuhuru_core_la_LIBADD+= @LIBCURL_LIBS@

install-exec-hook:
	-mkdir -p $(DESTDIR)$(localstatedir)/spool/uhuru
	-chmod 0777 $(DESTDIR)$(localstatedir)/spool/uhuru

