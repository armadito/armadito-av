bin_PROGRAMS=uhuru-daemon

uhuru_daemon_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src
uhuru_daemon_LDADD=$(top_builddir)/src/lib/libumwsu.la @GMODULE2_LIBS@ -lmagic
uhuru_daemon_SOURCES= \
main.c \
client.c \
client.h \
poll.c \
poll.h

