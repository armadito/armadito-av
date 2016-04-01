AUTOMAKE_OPTIONS=subdir-objects no-dependencies

bin_PROGRAMS=armadito-scan armadito-info

AM_CFLAGS=-I$(top_srcdir)/libarmadito/include -I$(top_srcdir)/libarmadito/include/config -I$(top_srcdir)/linux @GLIB2_CFLAGS@
LIBS=$(top_builddir)/libarmadito/src/libarmadito.la @GLIB2_LIBS@ -lmagic

armadito_scan_SOURCES=\
armadito-scan.c \
../../utils/getopt.c \
../../utils/getopt.h \
../daemon/ipc.c \
../daemon/ipc.h \
../net/netdefaults.h \
../net/tcpsockclient.c \
../net/tcpsockclient.h \
../net/unixsockclient.c \
../net/unixsockclient.h

armadito_info_SOURCES=\
armadito-info.c \
../../utils/getopt.c \
../../utils/getopt.h \
../daemon/ipc.c \
../daemon/ipc.h \
../net/netdefaults.h \
../net/tcpsockclient.c \
../net/tcpsockclient.h \
../net/unixsockclient.c \
../net/unixsockclient.h

armadito_info_CFLAGS=$(AM_CFLAGS) @LIBXML2_CFLAGS@
armadito_info_LDADD=$(LIBS) @LIBXML2_LIBS@
