AUTOMAKE_OPTIONS=subdir-objects

sbin_PROGRAMS=armadito-scand

AM_CPPFLAGS= -DLOCALSTATEDIR=\"$(localstatedir)\"
armadito_scand_CFLAGS=-I$(top_srcdir)/libarmadito/include -I$(top_srcdir) -I$(top_srcdir)/arch/linux -I$(top_srcdir)/json/ui -I$(top_srcdir)/libarmadito/src -DA6O_MHD_CONTENT_ROOT_DIR='"$(datadir)/armadito"' @GLIB2_CFLAGS@ @GIO2_CFLAGS@ @GTHREAD2_CFLAGS@ @LIBJANSSON_CFLAGS@ 
armadito_scand_LDADD=$(top_builddir)/libarmadito/src/libarmadito.la $(top_builddir)/json/ui/libarmadito_json.la @GLIB2_LIBS@ @GIO2_LIBS@ @GTHREAD2_LIBS@ @LIBJANSSON_LIBS@

armadito_scand_SOURCES= \
main.c \
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
