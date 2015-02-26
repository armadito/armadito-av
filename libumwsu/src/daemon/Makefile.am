bin_PROGRAMS=uhuru-daemon

uhuru_daemon_CFLAGS=-I$(top_srcdir)/include
uhuru_daemon_LDADD=$(top_builddir)/src/libumwsu.la @GMODULE2_LIBS@ -lmagic
uhuru_daemon_SOURCES=main.c
