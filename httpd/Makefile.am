sbin_PROGRAMS=armadito-httpd

AM_CPPFLAGS=-I$(top_srcdir) 
armadito_httpd_CFLAGS=-DA6O_MHD_CONTENT_ROOT_DIR='"$(datadir)/armadito"' @GLIB2_CFLAGS@ @GIO2_CFLAGS@ @GTHREAD2_CFLAGS@ @LIBJANSSON_CFLAGS@ @LIBMICROHTTPD_CFLAGS@ 
armadito_httpd_LDADD=@GLIB2_LIBS@ @GIO2_LIBS@ @GTHREAD2_LIBS@ @LIBJANSSON_LIBS@ @LIBMICROHTTPD_LIBS@ -lmagic

armadito_httpd_SOURCES= \
main.c \
httpd.c \
httpd.h \
log.c \
log.h \
apihandler.c \
apihandler.h \
api.c \
api.h 


