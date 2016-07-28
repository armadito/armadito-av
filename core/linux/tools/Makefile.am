AUTOMAKE_OPTIONS=subdir-objects no-dependencies

AM_CPPFLAGS= -I$(top_srcdir)/libarmadito/include/config

bin_PROGRAMS=armadito-scan

AM_CFLAGS= @LIBCURL_CFLAGS@ @LIBJSONC_CFLAGS@ @GLIB2_CFLAGS@
LIBS=@LIBCURL_LIBS@ @LIBJSONC_LIBS@ @GLIB2_LIBS@

armadito_scan_SOURCES=\
armadito-scan.c \
apiclient.c

#armadito_info_SOURCES=\
#armadito-info.c \
#../daemon/ipc.c \
#../daemon/ipc.h \
#../net/netdefaults.h \
#../net/unixsockclient.c \
#../net/unixsockclient.h
