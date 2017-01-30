sbin_PROGRAMS=armadito-httpd

AM_CPPFLAGS=$(PTHREAD_CFLAGS) -I$(top_srcdir) -I$(top_srcdir)/libmodule/include -I$(top_srcdir)/libcore/include -I$(top_srcdir)/librpc/include -I$(top_srcdir)/librpc/jrpc/include -I$(top_srcdir)/arch/linux
armadito_httpd_CFLAGS=-DA6O_MHD_CONTENT_ROOT_DIR='"$(datadir)/armadito"' @GLIB2_CFLAGS@ @GIO2_CFLAGS@ @GTHREAD2_CFLAGS@ @LIBJANSSON_CFLAGS@ @LIBMICROHTTPD_CFLAGS@ 
armadito_httpd_LDADD=$(PTHREAD_CFLAGS) $(top_builddir)/librpc/librpc.a $(top_builddir)/librpc/jrpc/libjrpc.a $(top_builddir)/libcore/libcore.a $(top_builddir)/libmodule/libarmadito.la @GLIB2_LIBS@ @GIO2_LIBS@ @GTHREAD2_LIBS@ @LIBJANSSON_LIBS@ @LIBMICROHTTPD_LIBS@ -lmagic

armadito_httpd_SOURCES= \
main.c \
httpd.c \
httpd.h \
log.c \
log.h \
apiclient.c \
apiclient.h \
apihandler.c \
apihandler.h \
api.c \
api.h \
../arch/linux/net/unixsockclient.c

