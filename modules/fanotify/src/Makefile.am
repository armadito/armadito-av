modulesdir=$(libdir)/libuhuru/modules
modules_LTLIBRARIES=fanotify.la

fanotify_la_SOURCES=fanotify.c fawatch.c fawatch.h

AM_CPPFLAGS= 
fanotify_la_LDFLAGS= -module -avoid-version
fanotify_la_CFLAGS=
fanotify_la_LIBADD=
fanotify_la_CFLAGS+= @LIBUHURU_CFLAGS@
fanotify_la_LIBADD+= @LIBUHURU_LIBS@
fanotify_la_CFLAGS+= @GLIB2_CFLAGS@
fanotify_la_LIBADD+= @GLIB2_LIBS@

install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/fanotify.la "$(DESTDIR)$(modulesdir)"/fanotify.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/fanotify.la "$(DESTDIR)$(modulesdir)"/fanotify.a
