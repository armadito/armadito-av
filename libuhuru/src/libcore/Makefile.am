AUTOMAKE_OPTIONS=subdir-objects

lib_LTLIBRARIES=libuhuru_core.la

libuhuru_core_la_SOURCES= \
action.c \
conf.c \
conf.h \
confparser.c \
confparser.h \
error.c \
filectx.c \
info.c \
log.c \
module.c \
modulep.h \
ondemand.c \
ondemandp.h \
report.c \
reportp.h \
scan.c \
scanp.h \
scanconf.c \
status.c \
statusp.h \
stdpaths.c \
uhuru.c \
uhurup.h \
builtin-modules/alert.c \
builtin-modules/alert.h \
builtin-modules/ondemandmod.c \
builtin-modules/ondemandmod.h \
builtin-modules/quarantine.c \
builtin-modules/quarantine.h \
../os/linux/dir.c \
../os/linux/file.c \
../os/linux/mimetype.c \
../os/linux/stdpaths.c 

AM_CPPFLAGS=-DUSE_FILE_DESCRIPTORS -DLIBUHURU_MODULES_PATH='"$(pkglibdir)/modules"' -DLIBUHURU_CONF_DIR='"$(sysconfdir)/uhuru"' -DLIBUHURU_BASES_DIR='"$(localstatedir)/lib/uhuru/bases"' -DLIBUHURU_TMP_DIR='"$(localstatedir)/lib/uhuru/tmp"' 
# removed the following -DG_LOG_DOMAIN='"uhuru"' which is remaining from glib log
libuhuru_core_la_CFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/include/config -I$(top_srcdir)/src
#libuhuru_core_la_LIBADD= $(top_builddir)/src/os/linux/dir.$(OBJEXT) $(top_builddir)/src/os/linux/file.$(OBJEXT) $(top_builddir)/src/os/linux/mimetype.$(OBJEXT)
libuhuru_core_la_LIBADD= 

libuhuru_core_la_CFLAGS+= @GLIB2_CFLAGS@
libuhuru_core_la_LIBADD+= @GLIB2_LIBS@

libuhuru_core_la_CFLAGS+= @GMODULE2_CFLAGS@
libuhuru_core_la_LIBADD+= @GMODULE2_LIBS@

libuhuru_core_la_CFLAGS+= @GTHREAD2_CFLAGS@
libuhuru_core_la_LIBADD+= @GTHREAD2_LIBS@

libuhuru_core_la_CFLAGS+= @LIBXML2_CFLAGS@
libuhuru_core_la_LIBADD+= @LIBXML2_LIBS@

#libuhuru_core_la_CFLAGS+= @LIBCURL_CFLAGS@
#libuhuru_core_la_LIBADD+= @LIBCURL_LIBS@

install-exec-hook:
	-mkdir -p $(DESTDIR)$(localstatedir)/spool/uhuru
	-chmod 0777 $(DESTDIR)$(localstatedir)/spool/uhuru
	-mkdir -p $(DESTDIR)$(localstatedir)/lib/uhuru/tmp

