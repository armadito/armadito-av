AUTOMAKE_OPTIONS=subdir-objects

modulesdir=$(libdir)/armadito/modules
modules_LTLIBRARIES=moduleH1.la

moduleH1_la_SOURCES= \
module/moduleH1.c \
module/h1-static.h \
lib/include/PEUtils.h \
lib/include/databases.h \
lib/include/distances.h \
lib/include/elfReader.h \
lib/include/gestionPE.h \
lib/include/kNN.h \
lib/include/miniz.h \
lib/include/util.h \
lib/include/model.h \
lib/include/table.h \
lib/include/tfidfDetection.h \
lib/include/h1-errors.h \
lib/include/vector.h \
lib/include/windowsTypes.h \
lib/include/osdeps.h \
lib/src/h1-static.c \
lib/src/databases.c \
lib/src/distances.c \
lib/src/elfReader.c \
lib/src/gestionPE.c \
lib/src/kNN.c \
lib/src/miniz.c \
lib/src/model.c \
lib/src/table.c \
lib/src/linux/osdeps.c \
lib/src/tfidfDetection.c \
lib/src/h1-errors.c \
lib/src/vector.c 

AM_CPPFLAGS= -DMODULEH1_DBDIR='"@MODULEH1_DBDIR@"'
moduleH1_la_LDFLAGS= -module -avoid-version -export-symbols "$(srcdir)/moduleH1.sym"
moduleH1_la_CFLAGS= -I$(srcdir)/lib/include
moduleH1_la_LIBADD=
moduleH1_la_CFLAGS+= @LIBARMADITO_CFLAGS@
moduleH1_la_LIBADD+= @LIBARMADITO_LIBS@

install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/moduleH1.la "$(DESTDIR)$(modulesdir)"/moduleH1.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/moduleH1.la "$(DESTDIR)$(modulesdir)"/moduleH1.a
