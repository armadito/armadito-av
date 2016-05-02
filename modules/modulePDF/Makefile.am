AUTOMAKE_OPTIONS=subdir-objects

modulesdir=$(libdir)/armadito/modules
modules_LTLIBRARIES=modulePDF.la

modulePDF_la_SOURCES= \
modulePDF.c \
modulePDF.h \
uhurupdf/module7/src/osdeps.c \
uhurupdf/module7/src/filters.c \
uhurupdf/module7/src/pdfAnalysis.c \
uhurupdf/module7/src/pdfParsing.c \
uhurupdf/module7/src/pdfStructAnalysis.c \
uhurupdf/module7/src/utils.c \
uhurupdf/module7/includes/filters.h \
uhurupdf/module7/includes/osdeps.h \
uhurupdf/module7/includes/pdfAnalyzer.h

AM_CPPFLAGS= 
modulePDF_la_LDFLAGS= -module -avoid-version -export-symbols "$(srcdir)/modulePDF.sym"
modulePDF_la_CFLAGS= -I$(srcdir)/uhurupdf/module7/includes
modulePDF_la_LIBADD=
modulePDF_la_CFLAGS+= @LIBARMADITO_CFLAGS@ 
modulePDF_la_LIBADD+= @LIBARMADITO_LIBS@


install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/modulePDF.la "$(DESTDIR)$(modulesdir)"/modulePDF.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/modulePDF.la "$(DESTDIR)$(modulesdir)"/modulePDF.a
