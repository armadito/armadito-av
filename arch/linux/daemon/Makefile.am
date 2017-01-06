AUTOMAKE_OPTIONS=subdir-objects

sbin_PROGRAMS=armadito-scand

AM_CPPFLAGS=$(PTHREAD_CFLAGS) -DLOCALSTATEDIR=\"$(localstatedir)\"
armadito_scand_CFLAGS=-I$(top_srcdir) -I$(top_srcdir)/libmodule/include -I$(top_srcdir)/libcore/include -I$(top_srcdir)/libjrpc/include -I$(top_srcdir)/arch/linux @GLIB2_CFLAGS@ @GIO2_CFLAGS@ @GTHREAD2_CFLAGS@ @GMODULE2_CFLAGS@ @LIBJANSSON_CFLAGS@ 
armadito_scand_LDADD=$(PTHREAD_CFLAGS) $(top_builddir)/libcore/libcore.a $(top_builddir)/libmodule/libarmadito.la $(top_builddir)/libjrpc/libjrpc.a $(PTHREAD_LIBS) @GLIB2_LIBS@ @GIO2_LIBS@ @GTHREAD2_LIBS@ @GMODULE2_LIBS@ @LIBJANSSON_LIBS@ -lmagic

armadito_scand_SOURCES= \
main.c \
log.c \
log.h \
server.c \
server.h \
daemonize.c \
daemonize.h \
unixsockserver.c \
unixsockserver.h \
../net/netdefaults.h

install-exec-hook:
	-mkdir -p $(DESTDIR)$(localstatedir)/run
