AUTOMAKE_OPTIONS=subdir-objects

lib_LTLIBRARIES=libarmadito.la

libarmadito_la_SOURCES= \
armadito.c \
armaditop.h \
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
linux/os/dir.c \
linux/os/file.c \
linux/os/mimetype.c \
linux/os/stdpaths.c \
linux/builtin-modules/alert.c \
linux/builtin-modules/alert.h \
linux/builtin-modules/ondemandmod.c \
linux/builtin-modules/ondemandmod.h \
linux/builtin-modules/quarantine.c \
linux/builtin-modules/quarantine.h 

AM_CPPFLAGS=-DUSE_FILE_DESCRIPTORS -DLIBARMADITO_MODULES_PATH='"$(pkglibdir)/modules"' -DLIBARMADITO_CONF_DIR='"$(sysconfdir)/armadito"' -DLIBARMADITO_BASES_DIR='"$(localstatedir)/lib/armadito/bases"' -DLIBARMADITO_TMP_DIR='"$(localstatedir)/lib/armadito/tmp"' 
libarmadito_la_CFLAGS=-I$(top_srcdir)/libarmadito/include -I$(top_srcdir)/libarmadito/include/config -I$(top_srcdir)/libarmadito/src/linux
libarmadito_la_LIBADD= 

libarmadito_la_CFLAGS+= @GLIB2_CFLAGS@
libarmadito_la_LIBADD+= @GLIB2_LIBS@

libarmadito_la_CFLAGS+= @GMODULE2_CFLAGS@
libarmadito_la_LIBADD+= @GMODULE2_LIBS@

libarmadito_la_CFLAGS+= @GTHREAD2_CFLAGS@
libarmadito_la_LIBADD+= @GTHREAD2_LIBS@

libarmadito_la_CFLAGS+= @LIBXML2_CFLAGS@
libarmadito_la_LIBADD+= @LIBXML2_LIBS@

install-exec-hook:
	-mkdir -p $(DESTDIR)$(localstatedir)/spool/armadito
	-chmod 0777 $(DESTDIR)$(localstatedir)/spool/armadito
	-mkdir -p $(DESTDIR)$(localstatedir)/lib/armadito/tmp

