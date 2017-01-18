AUTOMAKE_OPTIONS=subdir-objects no-dependencies

bin_PROGRAMS= armadito-info #armadito-scan

AM_CFLAGS=$(PTHREAD_CFLAGS) -I$(top_srcdir) -I$(top_srcdir)/libmodule/include -I$(top_srcdir)/libcore/include -I$(top_srcdir)/librpc/include -I$(top_srcdir)/librpc/jrpc/include -I$(top_srcdir)/arch/linux @LIBJANSSON_CFLAGS@ 
LIBS=$(PTHREAD_CFLAGS) $(top_builddir)/librpc/librpc.a $(top_builddir)/librpc/jrpc/libjrpc.a $(top_builddir)/libcore/libcore.a $(top_builddir)/libmodule/libarmadito.la $(PTHREAD_LIBS) @GLIB2_LIBS@ @GIO2_LIBS@ @GTHREAD2_LIBS@ @GMODULE2_LIBS@ @LIBJANSSON_LIBS@ -lmagic

#armadito_scan_SOURCES= armadito-scan.c

armadito_info_SOURCES= armadito-info.c ../arch/linux/net/unixsockclient.c

