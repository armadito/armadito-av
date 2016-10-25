AUTOMAKE_OPTIONS=subdir-objects

lib_LTLIBRARIES=libarmadito.la

libarmadito_la_SOURCES= \
confval.c \
log.c \
stdpaths.c \
string.c

AM_CPPFLAGS= \
	-DLIBARMADITO_MODULES_PATH='"$(libdir)/armadito/modules"' \
	-DLIBARMADITO_CONF_DIR='"$(sysconfdir)/armadito"' \
	-DLIBARMADITO_BASES_DIR='"$(localstatedir)/lib/armadito/bases"' \
	-DLIBARMADITO_TMP_DIR='"$(localstatedir)/lib/armadito/tmp"' 

libarmadito_la_CFLAGS=-I$(top_srcdir)/libmodule/include -I$(top_srcdir) 
libarmadito_la_LIBADD= 

libarmadito_la_CFLAGS+= @GLIB2_CFLAGS@
libarmadito_la_LIBADD+= @GLIB2_LIBS@

libarmadito_la_CFLAGS+= @GMODULE2_CFLAGS@
libarmadito_la_LIBADD+= @GMODULE2_LIBS@

libarmadito_la_CFLAGS+= @GTHREAD2_CFLAGS@
libarmadito_la_LIBADD+= @GTHREAD2_LIBS@

armaditoincludedir=$(includedir)/libarmadito

armaditoinclude_HEADERS= \
include/libarmadito/armadito.h \
include/libarmadito/confval.h \
include/libarmadito/info.h \
include/libarmadito/log.h \
include/libarmadito/module.h \
include/libarmadito/status.h \
include/libarmadito/stdpaths.h \
include/libarmadito/string.h 

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=libarmadito.pc

EDIT = sed \
        -e 's,@bindir\@,$(bindir),g' \
        -e 's,@sbindir\@,$(sbindir),g' \
        -e 's,@includedir\@,$(includedir),g' \
        -e 's,@libdir\@,$(libdir),g' \
        -e 's,@PACKAGE_VERSION\@,$(PACKAGE_VERSION),g' 

%.pc: Makefile $(srcdir)/%.pc.in
	rm -f $*.pc $*.pc.tmp
	$(EDIT) $(srcdir)/$*.pc.in >$*.pc.tmp
	mv $*.pc.tmp $*.pc

EXTRA_DIST=libarmadito.pc.in

CLEANFILES=libarmadito.pc
