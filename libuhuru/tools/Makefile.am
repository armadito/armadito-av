bin_PROGRAMS=uhuru-scan uhuru-info

uhuru_scan_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src
uhuru_scan_LDADD=$(top_builddir)/src/lib/libuhuru.la @GMODULE2_LIBS@ -lmagic
uhuru_scan_SOURCES=uhuru-scan.c

uhuru_info_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src @GMODULE2_CFLAGS@ @LIBXML2_CFLAGS@
uhuru_info_LDADD=$(top_builddir)/src/lib/libuhuru.la @GMODULE2_LIBS@ @LIBXML2_LIBS@ -lmagic
uhuru_info_SOURCES=uhuru-info.c
