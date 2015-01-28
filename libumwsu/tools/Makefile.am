bin_PROGRAMS=uhuru-scan

uhuru_scan_CFLAGS=-I$(top_srcdir)/include
uhuru_scan_LDADD=$(top_builddir)/src/libumwsu.la @GMODULE2_LIBS@ -lmagic
uhuru_scan_SOURCES=uhuru-scan.c

#dist_bin_SCRIPTS=umwsu-fake-server.py

check_PROGRAMS=uhuru-watch
uhuru_watch_CFLAGS=-I$(top_srcdir)/include
uhuru_watch_LDADD=$(top_builddir)/src/libumwsu.la @GMODULE2_LIBS@ -lmagic
uhuru_watch_SOURCES=uhuru-watch.c
