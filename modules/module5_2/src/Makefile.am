AUTOMAKE_OPTIONS=subdir-objects

#modulesdir=@LIBUMWSU_MODDIR@
modulesdir=$(libdir)/libumwsu/modules
modules_LTLIBRARIES=module5_2.la

ORG_SOURCES= \
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

dist_module5_2_la_SOURCES= module5_2.c $(ORG_SOURCES)
#nodist_module5_2_la_SOURCES= $(ORG_SOURCES)

AM_CPPFLAGS= -DMODULE5_2_DBDIR='"@MODULE5_2_DBDIR@"'
module5_2_la_LDFLAGS= -module -avoid-version
module5_2_la_CFLAGS= -I$(srcdir)/org
module5_2_la_LIBADD=
module5_2_la_CFLAGS+= @LIBUMWSU_CFLAGS@
module5_2_la_LIBADD+= @LIBUMWSU_LIBS@

install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/module5_2.la "$(DESTDIR)$(modulesdir)"/module5_2.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/module5_2.la "$(DESTDIR)$(modulesdir)"/module5_2.a
