AUTOMAKE_OPTIONS=subdir-objects

modulesdir=$(libdir)/libuhuru/modules
modules_LTLIBRARIES=on-access-linux.la

on_access_linux_la_SOURCES= \
onaccessmod.c \
monitor.c \
monitor.h \
mount.c \
mount.h 

on_access_linux_la_LDFLAGS= -module -avoid-version
on_access_linux_la_LIBADD= @GLIB2_LIBS@
on_access_linux_la_CFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/src -I$(top_srcdir)/src/linux @GLIB2_CFLAGS@

install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/on-access-linux.la "$(DESTDIR)$(modulesdir)"/on-access-linux.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/on-access-linux.la "$(DESTDIR)$(modulesdir)"/on-access-linux.a
