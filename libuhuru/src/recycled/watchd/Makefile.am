bin_PROGRAMS=uhuru-watch-daemon

uhuru_watch_daemon_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src @GMODULE2_CFLAGS@ @GIO2_CFLAGS@
uhuru_watch_daemon_LDADD=$(top_builddir)/src/lib/libuhuru.la @GMODULE2_LIBS@ @GIO2_LIBS@ -lmagic
uhuru_watch_daemon_SOURCES= \
main.c \
watch.h \
watch.c

EXTRA_DIST=uhuru-watch-daemon.desktop
