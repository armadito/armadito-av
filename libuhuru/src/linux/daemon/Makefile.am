AUTOMAKE_OPTIONS=subdir-objects

sbin_PROGRAMS=uhuru-scand

AM_CPPFLAGS= -DG_LOG_DOMAIN='"uhuru"' -DLOCALSTATEDIR=\"$(localstatedir)\"
uhuru_scand_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/include/config -I$(top_srcdir)/src @GLIB2_CFLAGS@ @GIO2_CFLAGS@ @GTHREAD2_CFLAGS@
uhuru_scand_LDADD=$(top_builddir)/src/libcore/libuhuru_core.la @GLIB2_LIBS@ @GIO2_LIBS@ @GTHREAD2_LIBS@ -lmagic

uhuru_scand_SOURCES= \
main.c \
client.c \
client.h \
ipc.c \
ipc.h \
log.c \
log.h \
server.c \
server.h \
daemonize.c \
daemonize.h \
unixsock.c \
unixsock.h \
tcpsock.c \
tcpsock.h \
../../utils/getopt.c \
../../utils/getopt.h 

EXTRA_DIST=uhuru-scan-daemon.desktop

install-exec-hook:
	-mkdir -p $(DESTDIR)$(localstatedir)/run
