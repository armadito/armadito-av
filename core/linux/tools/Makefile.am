AUTOMAKE_OPTIONS=subdir-objects no-dependencies

AM_CPPFLAGS= -I$(top_srcdir)/libarmadito/include/config

bin_PROGRAMS=armadito-scan armadito-info

AM_CFLAGS= @LIBCURL_CFLAGS@ @LIBJSONC_CFLAGS@ @GLIB2_CFLAGS@
LIBS=@LIBCURL_LIBS@ @LIBJSONC_LIBS@ @GLIB2_LIBS@

COMMON_SOURCES= apiclient.c apiclient.h jutil.c jutil.h

armadito_scan_SOURCES= armadito-scan.c $(COMMON_SOURCES)

armadito_info_SOURCES= armadito-info.c $(COMMON_SOURCES)
