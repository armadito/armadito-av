AUTOMAKE_OPTIONS=subdir-objects

bin_PROGRAMS=uhuru-scand

uhuru_scand_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/include/config -I$(top_srcdir)/src @GLIB2_CFLAGS@ @GIO2_CFLAGS@ @GTHREAD2_CFLAGS@
uhuru_scand_LDADD=$(top_builddir)/src/libcore/libuhuru_core.la $(top_builddir)/src/libipc/libuhuru_ipc.la @GLIB2_LIBS@ @GIO2_LIBS@ @GTHREAD2_LIBS@ -lmagic

uhuru_scand_SOURCES= \
main.c \
client.c \
client.h \
server.c \
server.h \
daemonize.c \
daemonize.h \
unixsock.c \
unixsock.h \
../utils/getopt.c \
../utils/getopt.h \
../utils/tcpsock.c \
../utils/tcpsock.h 

EXTRA_DIST=uhuru-scan-daemon.desktop
