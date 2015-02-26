bin_PROGRAMS=uhuru-scan uhuru-watch

uhuru_scan_CFLAGS=-I$(top_srcdir)/include
uhuru_scan_LDADD=$(top_builddir)/src/lib/libumwsu.la @GMODULE2_LIBS@ -lmagic
uhuru_scan_SOURCES=uhuru-scan.c

uhuru_watch_CFLAGS=-I$(top_srcdir)/include
uhuru_watch_LDADD=$(top_builddir)/src/lib/libumwsu.la @GMODULE2_LIBS@ -lmagic
uhuru_watch_SOURCES=uhuru-watch.c
