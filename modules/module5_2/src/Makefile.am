AUTOMAKE_OPTIONS=subdir-objects

modulesdir=$(libdir)/armadito/modules
modules_LTLIBRARIES=module5_2.la

module5_2_la_SOURCES= \
module/module5_2.c \
module/UhuruStatic.h \
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
lib/include/uh_errors.h \
lib/include/vector.h \
lib/include/windowsTypes.h \
lib/include/osdeps.h \
lib/src/UhuruStatic.c \
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
lib/src/uh_errors.c \
lib/src/vector.c 

AM_CPPFLAGS= -DMODULE5_2_DBDIR='"@MODULE5_2_DBDIR@"'
module5_2_la_LDFLAGS= -module -avoid-version -export-symbols "$(srcdir)/module5_2.sym"
module5_2_la_CFLAGS= -I$(srcdir)/lib/include
module5_2_la_LIBADD=
module5_2_la_CFLAGS+= @LIBARMADITO_CFLAGS@
module5_2_la_LIBADD+= @LIBARMADITO_LIBS@

install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/module5_2.la "$(DESTDIR)$(modulesdir)"/module5_2.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/module5_2.la "$(DESTDIR)$(modulesdir)"/module5_2.a
