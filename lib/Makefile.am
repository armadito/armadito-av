AUTOMAKE_OPTIONS=subdir-objects

lib_LTLIBRARIES=libarmadito.la

libarmadito_la_SOURCES= \
log.c 

AM_CPPFLAGS=
libarmadito_la_CFLAGS=-I$(top_srcdir)/lib/include -I$(top_srcdir) #-I$(top_srcdir)/libarmadito/src/linux
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
include/libarmadito/log.h

# to be recycled
#libarmadito/action.h \
#libarmadito/conf.h \
#libarmadito/error.h \
#libarmadito/filectx.h \
#libarmadito/handle.h \
#libarmadito/info.h \
#libarmadito/log.h \
#libarmadito/module.h \
#libarmadito/notify.h \
#libarmadito/ondemand.h \
#libarmadito/scan.h \
#libarmadito/scanconf.h \
#libarmadito/status.h \
#libarmadito/stdpaths.h 

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
