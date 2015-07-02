bin_PROGRAMS=uhuru-scan-daemon

uhuru_scan_daemon_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src @GMODULE2_CFLAGS@ @GIO2_CFLAGS@
uhuru_scan_daemon_LDADD=$(top_builddir)/src/lib/libuhuru.la @GMODULE2_LIBS@ @GIO2_LIBS@ -lmagic
uhuru_scan_daemon_SOURCES= \
main.c \
client.c \
client.h \
server.c \
server.h

EXTRA_DIST=uhuru-scan-daemon.desktop
