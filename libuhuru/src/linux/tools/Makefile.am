AUTOMAKE_OPTIONS=subdir-objects

bin_PROGRAMS=uhuru-scan # uhuru-info later

uhuru_scan_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src -I$(top_srcdir)/src/linux @GLIB2_CFLAGS@
uhuru_scan_LDADD=@GLIB2_LIBS@
uhuru_scan_SOURCES=\
uhuru-scan.c \
../../utils/getopt.c \
../../utils/getopt.h \
../daemon/ipc.c \
../daemon/ipc.h \
../daemon/tcpsock.c \
../daemon/tcpsock.h

#uhuru_info_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src @GLIB2_CFLAGS@ @LIBXML2_CFLAGS@
#uhuru_info_LDADD=$(top_builddir)/src/lib/libuhuru.la @GLIB2_LIBS@ @LIBXML2_LIBS@
#uhuru_info_SOURCES=uhuru-info.c
