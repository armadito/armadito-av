AUTOMAKE_OPTIONS=subdir-objects

modulesdir=$(libdir)/armadito/modules
modules_LTLIBRARIES=on-access-linux.la

on_access_linux_la_SOURCES= \
famonitor.c \
famonitor.h \
imonitor.c \
imonitor.h \
onaccessmod.c \
onaccessmod.h \
monitor.c \
monitor.h \
mount.c \
mount.h \
response.c \
response.h \
queue.c \
queue.h \
stamp.h \
watchdog.c \
watchdog.h 

on_access_linux_la_LDFLAGS= -module -avoid-version
on_access_linux_la_LIBADD= @GLIB2_LIBS@ @GIO2_LIBS@
on_access_linux_la_CFLAGS= -I$(top_srcdir)/libmodule/include -I$(top_srcdir) @GLIB2_CFLAGS@ @GIO2_CFLAGS@

install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/on-access-linux.la "$(DESTDIR)$(modulesdir)"/on-access-linux.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/on-access-linux.la "$(DESTDIR)$(modulesdir)"/on-access-linux.a

uninstall-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/on-access-linux.so
