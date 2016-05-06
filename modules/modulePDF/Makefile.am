AUTOMAKE_OPTIONS=subdir-objects

modulesdir=$(libdir)/armadito/modules
modules_LTLIBRARIES=modulePDF.la

modulePDF_la_SOURCES= \
modulePDF.c \
modulePDF.h \
armaditopdf/module7/includes/pdfAnalyzer.h \
armaditopdf/module7/includes/filters.h \
armaditopdf/module7/includes/osdeps.h \
armaditopdf/module7/src/osdeps.c \
armaditopdf/module7/src/filters.c \
armaditopdf/module7/src/pdfAnalysis.c \
armaditopdf/module7/src/pdfParsing.c \
armaditopdf/module7/src/pdfStructAnalysis.c \
armaditopdf/module7/src/pdfObjectsAnalysis.c \
armaditopdf/module7/src/utils.c

AM_CPPFLAGS= 
modulePDF_la_LDFLAGS= -module -avoid-version -export-symbols "$(srcdir)/modulePDF.sym"
modulePDF_la_CFLAGS= -I$(srcdir)/armaditopdf/module7/includes
modulePDF_la_LIBADD=
modulePDF_la_CFLAGS+= @LIBARMADITO_CFLAGS@ 
modulePDF_la_LIBADD+= @LIBARMADITO_LIBS@


install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/modulePDF.la "$(DESTDIR)$(modulesdir)"/modulePDF.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/modulePDF.la "$(DESTDIR)$(modulesdir)"/modulePDF.a
