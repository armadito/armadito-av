AUTOMAKE_OPTIONS=subdir-objects

sbin_PROGRAMS=uhuru-scand

AM_CPPFLAGS= -DG_LOG_DOMAIN='"uhuru"' -DLOCALSTATEDIR=\"$(localstatedir)\"
uhuru_scand_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/include/config -I$(top_srcdir)/src -I$(top_srcdir)/src/linux -I$(top_srcdir)/src/json/ui @GLIB2_CFLAGS@ @GIO2_CFLAGS@ @GTHREAD2_CFLAGS@ @LIBJSONC_CFLAGS@
uhuru_scand_LDADD=$(top_builddir)/src/libcore/libuhuru_core.la $(top_builddir)/src/json/ui/libuhuru_json.la @GLIB2_LIBS@ @GIO2_LIBS@ @GTHREAD2_LIBS@ @LIBJSONC_LIBS@ -lmagic

uhuru_scand_SOURCES= \
main.c \
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
tcpsockserver.c \
tcpsockserver.h \
../net/netdefaults.h \
../../utils/getopt.c \
../../utils/getopt.h 

EXTRA_DIST=uhuru-scan-daemon.desktop

install-exec-hook:
	-mkdir -p $(DESTDIR)$(localstatedir)/run
