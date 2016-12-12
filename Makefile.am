if COND_MAN
OPT_MAN_DIR=man
endif

SUBDIRS=conf libmodule libcore libjrpc httpd $(OPT_MAN_DIR)

TAGDIRS=\
$(top_srcdir)/arch \
$(top_srcdir)/conf \
$(top_srcdir)/httpd \
$(top_srcdir)/libcore \
$(top_srcdir)/libjrpc \
$(top_srcdir)/libmodule \
$(top_srcdir)/man \
$(top_srcdir)/tests \
$(top_srcdir)/tools 

TAGS:
	(for d in $(TAGDIRS) ; do find $$d -name '*.c' -o -name '*.h' -o -name '*.txt' -o -name 'Makefile.am' -o -name '*.pc.in' ; done ) | etags -
.PHONY: TAGS


EXTRA_DIST=armadito-config.h armadito-config-win32.h
