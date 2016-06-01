AUTOMAKE_OPTIONS=subdir-objects no-dependencies

bin_PROGRAMS= armadito-info armadito-scan

AM_CFLAGS=-I$(top_srcdir)/libarmadito/include -I$(top_srcdir)/libarmadito/include/config -I$(top_srcdir)/linux @GLIB2_CFLAGS@
LIBS=$(top_builddir)/libarmadito/src/libarmadito.la @GLIB2_LIBS@ -lmagic

armadito_info_SOURCES=\
armadito-info.c \
../daemon/ipc.c \
../daemon/ipc.h \
../net/netdefaults.h \
../net/unixsockclient.c \
../net/unixsockclient.h

armadito_scan_SOURCES=\
armadito-scan.c \
../daemon/ipc.c \
../daemon/ipc.h \
../net/netdefaults.h \
../net/unixsockclient.c \
../net/unixsockclient.h

armadito_info_CFLAGS=$(AM_CFLAGS) @LIBXML2_CFLAGS@
armadito_info_LDADD=$(LIBS) @LIBXML2_LIBS@
