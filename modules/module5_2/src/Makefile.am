AUTOMAKE_OPTIONS=subdir-objects

modulesdir=$(libdir)/libuhuru/modules
modules_LTLIBRARIES=module5_2.la

module5_2_la_SOURCES= \
module5_2.c \
module5_2_sources/UhuruStatic/include/PEUtils.h \
module5_2_sources/UhuruStatic/src/databases.c \
module5_2_sources/UhuruStatic/include/databases.h \
module5_2_sources/UhuruStatic/src/distances.c \
module5_2_sources/UhuruStatic/include/distances.h \
module5_2_sources/UhuruStatic/src/elfReader.c \
module5_2_sources/UhuruStatic/include/elfReader.h \
module5_2_sources/UhuruStatic/src/gestionPE.c \
module5_2_sources/UhuruStatic/include/gestionPE.h \
module5_2_sources/UhuruStatic/src/kNN.c \
module5_2_sources/UhuruStatic/include/kNN.h \
module5_2_sources/UhuruStatic/src/miniz.c \
module5_2_sources/UhuruStatic/include/miniz.h \
module5_2_sources/UhuruStatic/src/model.c \
module5_2_sources/UhuruStatic/include/model.h \
module5_2_sources/UhuruStatic/src/table.c \
module5_2_sources/UhuruStatic/include/table.h \
module5_2_sources/UhuruStatic/src/tfidfDetection.c \
module5_2_sources/UhuruStatic/include/tfidfDetection.h \
module5_2_sources/UhuruStatic/src/uh_errors.c \
module5_2_sources/UhuruStatic/include/uh_errors.h \
module5_2_sources/UhuruStatic/src/UhuruStatic.c \
module5_2_sources/UhuruStatic/resources/UhuruStatic.h \
module5_2_sources/UhuruStatic/src/utils.c \
module5_2_sources/UhuruStatic/include/utils.h \
module5_2_sources/UhuruStatic/src/vector.c \
module5_2_sources/UhuruStatic/include/vector.h \
module5_2_sources/UhuruStatic/include/windowsTypes.h

AM_CPPFLAGS= -DMODULE5_2_DBDIR='"@MODULE5_2_DBDIR@"'
module5_2_la_LDFLAGS= -module -avoid-version
module5_2_la_CFLAGS= -I$(srcdir)/module5_2_sources/UhuruStatic/include -I$(srcdir)/module5_2_sources/UhuruStatic/resources
module5_2_la_LIBADD=
module5_2_la_CFLAGS+= @LIBUHURU_CFLAGS@
module5_2_la_LIBADD+= @LIBUHURU_LIBS@

install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/module5_2.la "$(DESTDIR)$(modulesdir)"/module5_2.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/module5_2.la "$(DESTDIR)$(modulesdir)"/module5_2.a
