AUTOMAKE_OPTIONS=subdir-objects

sbin_PROGRAMS=armadito-scand

AM_CPPFLAGS= -DLOCALSTATEDIR=\"$(localstatedir)\"
armadito_scand_CFLAGS=-I$(top_srcdir)/libarmadito/include -I$(top_srcdir)/libarmadito/include/config -I$(top_srcdir)/linux -I$(top_srcdir)/json/ui -I$(top_srcdir)/libarmadito/src -DA6O_MHD_CONTENT_ROOT_DIR='"$(datadir)/armadito"' @GLIB2_CFLAGS@ @GIO2_CFLAGS@ @GTHREAD2_CFLAGS@ @LIBJSONC_CFLAGS@ @LIBMICROHTTPD_CFLAGS@
armadito_scand_LDADD=$(top_builddir)/libarmadito/src/libarmadito.la $(top_builddir)/json/ui/libarmadito_json.la @GLIB2_LIBS@ @GIO2_LIBS@ @GTHREAD2_LIBS@ @LIBJSONC_LIBS@  @LIBMICROHTTPD_LIBS@ -lmagic

armadito_scand_SOURCES= \
main.c \
httpd.c \
httpd.h \
ipc.c \
ipc.h \
ipcclient.c \
ipcclient.h \
jsonclient.c \
jsonclient.h \
log.c \
log.h \
server.c \
server.h \
daemonize.c \
daemonize.h \
unixsockserver.c \
unixsockserver.h \
../net/netdefaults.h 

install-exec-hook:
	-mkdir -p $(DESTDIR)$(localstatedir)/run
