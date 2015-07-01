if COND_DOC
OPT_DOC_DIR=doc
endif

SUBDIRS=conf include src tests pkgconfig tools $(OPT_DOC_DIR)

TAGDIRS=\
$(top_srcdir)/doc \
$(top_srcdir)/include \
$(top_srcdir)/pkgconfig \
$(top_srcdir)/src \
$(top_srcdir)/tests \
$(top_srcdir)/tools

TAGS:
	(for d in $(TAGDIRS) ; do find $$d -name '*.c' -o -name '*.h' -o -name '*.txt' -o -name 'Makefile.am' -o -name '*.pc.in' ; done ) | etags -
.PHONY: TAGS

