if COND_MAN
OPT_MAN_DIR=man
endif

SUBDIRS=conf libarmadito arch tools tests $(OPT_MAN_DIR)

TAGDIRS=\
$(top_srcdir)/arch \
$(top_srcdir)/conf \
$(top_srcdir)/json \
$(top_srcdir)/libarmadito \
$(top_srcdir)/man \
$(top_srcdir)/tools \
$(top_srcdir)/tests 

TAGS:
	(for d in $(TAGDIRS) ; do find $$d -name '*.c' -o -name '*.h' -o -name '*.txt' -o -name 'Makefile.am' -o -name '*.pc.in' ; done ) | etags -
.PHONY: TAGS


EXTRA_DIST=armadito-config.h armadito-config-win32.h
