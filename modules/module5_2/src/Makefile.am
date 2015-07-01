AUTOMAKE_OPTIONS=subdir-objects

# this does not work w.r.t. rpm packaging
#modulesdir=@LIBUMWSU_MODDIR@
modulesdir=$(libdir)/libumwsu/modules
modules_LTLIBRARIES=module5_2.la

module5_2_la_SOURCES= \
module5_2.c \
module5_2_sources/module.git/UhuruStatic/include/PEUtils.h \
module5_2_sources/module.git/UhuruStatic/src/databases.c \
module5_2_sources/module.git/UhuruStatic/include/databases.h \
module5_2_sources/module.git/UhuruStatic/src/distances.c \
module5_2_sources/module.git/UhuruStatic/include/distances.h \
module5_2_sources/module.git/UhuruStatic/src/elfReader.c \
module5_2_sources/module.git/UhuruStatic/include/elfReader.h \
module5_2_sources/module.git/UhuruStatic/src/gestionPE.c \
module5_2_sources/module.git/UhuruStatic/include/gestionPE.h \
module5_2_sources/module.git/UhuruStatic/src/kNN.c \
module5_2_sources/module.git/UhuruStatic/include/kNN.h \
module5_2_sources/module.git/UhuruStatic/src/miniz.c \
module5_2_sources/module.git/UhuruStatic/include/miniz.h \
module5_2_sources/module.git/UhuruStatic/src/model.c \
module5_2_sources/module.git/UhuruStatic/include/model.h \
module5_2_sources/module.git/UhuruStatic/src/table.c \
module5_2_sources/module.git/UhuruStatic/include/table.h \
module5_2_sources/module.git/UhuruStatic/src/tfidfDetection.c \
module5_2_sources/module.git/UhuruStatic/include/tfidfDetection.h \
module5_2_sources/module.git/UhuruStatic/src/uh_errors.c \
module5_2_sources/module.git/UhuruStatic/include/uh_errors.h \
module5_2_sources/module.git/UhuruStatic/src/UhuruStatic.c \
module5_2_sources/module.git/UhuruStatic/resources/UhuruStatic.h \
module5_2_sources/module.git/UhuruStatic/src/utils.c \
module5_2_sources/module.git/UhuruStatic/include/utils.h \
module5_2_sources/module.git/UhuruStatic/src/vector.c \
module5_2_sources/module.git/UhuruStatic/include/vector.h \
module5_2_sources/module.git/UhuruStatic/include/windowsTypes.h

LEFTOVER= \
org/databases.c \
org/databases.h \
org/distances.c \
org/distances.h \
org/elfReader.c \
org/elfReader.h \
org/errors.h \
org/kNN.c \
org/kNN.h \
org/main.c \
org/miniz.c \
org/miniz.h \
org/model.c \
org/model.h \
org/table.c \
org/table.h \
org/tfidfDetection.c \
org/tfidfDetection.h \
org/utils.h \
org/vector.c \
org/vector.h \
org/windowsTypes.h 

AM_CPPFLAGS= -DMODULE5_2_DBDIR='"@MODULE5_2_DBDIR@"'
module5_2_la_LDFLAGS= -module -avoid-version
module5_2_la_CFLAGS= -I$(srcdir)/module5_2_sources/module.git/UhuruStatic/include -I$(srcdir)/module5_2_sources/module.git/UhuruStatic/resources
module5_2_la_LIBADD=
module5_2_la_CFLAGS+= @LIBUMWSU_CFLAGS@
module5_2_la_LIBADD+= @LIBUMWSU_LIBS@

install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/module5_2.la "$(DESTDIR)$(modulesdir)"/module5_2.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/module5_2.la "$(DESTDIR)$(modulesdir)"/module5_2.a
