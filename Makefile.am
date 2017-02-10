if COND_MAN
OPT_MAN_DIR=man
endif

SUBDIRS=conf libmodule libcore librpc httpd arch tools tests $(OPT_MAN_DIR)

TAGDIRS=\
$(top_srcdir)/arch \
$(top_srcdir)/conf \
$(top_srcdir)/httpd \
$(top_srcdir)/libcore \
$(top_srcdir)/librpc \
$(top_srcdir)/libmodule \
$(top_srcdir)/man \
$(top_srcdir)/tests \
$(top_srcdir)/tools



.PHONY: TAGS coverage

TAGS:
	(for d in $(TAGDIRS) ; do find $$d -name '*.c' -o -name '*.h' -o -name '*.txt' -o -name 'Makefile.am' -o -name '*.pc.in' ; done ) | etags -

GCOV_FILES=$(shell for file in `find $(top_builddir) -name '*.gcda'`;do echo $$file; done)

coverage:
	@echo Collecting coverage data from $(top_builddir)
	mkdir -p coverage
	for d in $(basename $(GCOV_FILES)) ; do gcov -p $$d ; done
	mv -f *.gcov coverage

EXTRA_DIST=armadito-config.h armadito-config-win32.h
