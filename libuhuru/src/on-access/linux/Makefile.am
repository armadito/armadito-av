bin_PROGRAMS=uhuru-on-access-daemon

uhuru_on_access_daemon_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/include/config -I$(top_srcdir)/src @GLIB2_CFLAGS@ @GIO2_CFLAGS@ @GTHREAD2_CFLAGS@
uhuru_on_access_daemon_LDADD=$(top_builddir)/src/libcore/libuhuru_core.la $(top_builddir)/src/libipc/libuhuru_ipc.la @GLIB2_LIBS@ @GIO2_LIBS@ @GTHREAD2_LIBS@ -lmagic
uhuru_on_access_daemon_SOURCES= \
fanotify.c \
on-access.c 
