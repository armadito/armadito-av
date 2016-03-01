AUTOMAKE_OPTIONS=subdir-objects no-dependencies

bin_PROGRAMS=uhuru-scan uhuru-info

AM_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src -I$(top_srcdir)/src/linux @GLIB2_CFLAGS@
# $(top_builddir)/src/libcore/libuhuru_core.la should not be there... except for uhuru_log, which is used by ipc.c, which is a mistake!
LIBS=$(top_builddir)/src/libcore/libuhuru_core.la @GLIB2_LIBS@ -lmagic

uhuru_scan_SOURCES=\
uhuru-scan.c \
../../utils/getopt.c \
../../utils/getopt.h \
../daemon/ipc.c \
../daemon/ipc.h \
../net/netdefaults.h \
../net/tcpsockclient.c \
../net/tcpsockclient.h \
../net/unixsockclient.c \
../net/unixsockclient.h

uhuru_info_SOURCES=\
uhuru-info.c \
../../utils/getopt.c \
../../utils/getopt.h \
../daemon/ipc.c \
../daemon/ipc.h \
../net/netdefaults.h \
../net/tcpsockclient.c \
../net/tcpsockclient.h \
../net/unixsockclient.c \
../net/unixsockclient.h

uhuru_info_CFLAGS=$(AM_CFLAGS) @LIBXML2_CFLAGS@
uhuru_info_LDADD=$(LIBS) @LIBXML2_LIBS@
