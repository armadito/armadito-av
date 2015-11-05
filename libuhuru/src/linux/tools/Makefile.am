AUTOMAKE_OPTIONS=subdir-objects

bin_PROGRAMS=uhuru-scan uhuru-info

AM_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src -I$(top_srcdir)/src/linux @GLIB2_CFLAGS@
LIBS=@GLIB2_LIBS@

COMMON_SOURCES=\
../../utils/getopt.c \
../../utils/getopt.h \
../daemon/ipc.c \
../daemon/ipc.h \
../daemon/tcpsock.c \
../daemon/tcpsock.h \
../daemon/unixsock.c \
../daemon/unixsock.h

uhuru_scan_SOURCES=uhuru-scan.c $(COMMON_SOURCES)

uhuru_info_SOURCES=uhuru-info.c $(COMMON_SOURCES)

uhuru_info_CFLAGS=$(AM_CFLAGS) @LIBXML2_CFLAGS@
uhuru_info_LDADD=$(LIBS) @LIBXML2_LIBS@
