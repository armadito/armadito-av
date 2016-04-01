SUBDIRS=libarmadito linux

TAGDIRS=\
$(top_srcdir)/doc \
$(top_srcdir)/include \
$(top_srcdir)/pkgconfig \
$(top_srcdir)/src \
$(top_srcdir)/tests 

TAGS:
	(for d in $(TAGDIRS) ; do find $$d -name '*.c' -o -name '*.h' -o -name '*.txt' -o -name 'Makefile.am' -o -name '*.pc.in' ; done ) | etags -
.PHONY: TAGS


