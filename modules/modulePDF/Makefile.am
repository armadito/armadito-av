AUTOMAKE_OPTIONS=subdir-objects

modulesdir=$(libdir)/armadito/modules
modules_LTLIBRARIES=modulePDF.la

modulePDF_la_SOURCES= \
modulePDF.c \
modulePDF.h \
armadito-pdf/lib/includes/utils.h \
armadito-pdf/lib/includes/armaditopdf.h \
armadito-pdf/lib/includes/filters.h \
armadito-pdf/lib/includes/log.h \
armadito-pdf/lib/includes/osdeps.h \
armadito-pdf/lib/includes/pdfAnalysis.h \
armadito-pdf/lib/includes/pdfparsing.h \
armadito-pdf/lib/includes/pdfStructs.h \
armadito-pdf/lib/src/armaditopdf.c \
armadito-pdf/lib/src/filters.c \
armadito-pdf/lib/src/log.c \
armadito-pdf/lib/src/osdeps.c \
armadito-pdf/lib/src/pdfObjectsAnalysis.c \
armadito-pdf/lib/src/pdfParsing.c \
armadito-pdf/lib/src/pdfStructAnalysis.c \
armadito-pdf/lib/src/pdfStructs.c \
armadito-pdf/lib/src/utils.c


AM_CPPFLAGS= 
modulePDF_la_LDFLAGS= -module -avoid-version -export-symbols "$(srcdir)/modulePDF.sym"
modulePDF_la_CFLAGS= -I$(srcdir)/armadito-pdf/lib/includes
modulePDF_la_LIBADD=
modulePDF_la_CFLAGS+= @LIBARMADITO_CFLAGS@ 
modulePDF_la_LIBADD+= @LIBARMADITO_LIBS@


install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/modulePDF.la "$(DESTDIR)$(modulesdir)"/modulePDF.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/modulePDF.la "$(DESTDIR)$(modulesdir)"/modulePDF.a
