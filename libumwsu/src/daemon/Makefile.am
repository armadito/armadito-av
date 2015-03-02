bin_PROGRAMS=uhuru-daemon

uhuru_daemon_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src @GMODULE2_CFLAGS@
uhuru_daemon_LDADD=$(top_builddir)/src/lib/libumwsu.la @GMODULE2_LIBS@ -lmagic
uhuru_daemon_SOURCES= \
main.c \
server.c \
server.h

install-exec-hook:
	-mkdir -p $(DESTDIR)$(localstatedir)/run/
