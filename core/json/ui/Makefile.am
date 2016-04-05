# TESTING:
# nc -Ulk /tmp/.armadito-ihm | ./libarmadito/tests/testjsonprint1 
# echo -en '{ "av_request":"scan", "id":123, "params": {"path":"/home/francois/Bureau"}}\r\n\r\n' | nc -U /tmp/.armadito-daemon  | ./libarmadito/tests/testjsonprint1 

AUTOMAKE_OPTIONS=subdir-objects

noinst_LTLIBRARIES=libarmadito_json.la

libarmadito_json_la_SOURCES= \
jsonhandler.c \
jsonhandler.h \
jsonhandlerp.h \
conf.c \
conf.h \
debug.c \
debug.h \
quarantine-dummy.c \
quarantine.h \
scan.c \
scan.h \
state.c \
state.h \
ui.h \
ui-linux.c \
../../linux/net/unixsockclient.c \
../../linux/net/unixsockclient.h

libarmadito_json_la_CFLAGS= -I$(top_srcdir)/libarmadito/include -I$(top_srcdir)/libarmadito/include/config -I$(top_srcdir)/linux
libarmadito_json_la_LIBADD= 

libarmadito_json_la_CFLAGS+= @LIBJSONC_CFLAGS@
libarmadito_json_la_LIBADD+= @LIBJSONC_LIBS@
libarmadito_json_la_CFLAGS+= @GLIB2_CFLAGS@
libarmadito_json_la_LIBADD+= @GLIB2_LIBS@

EXTRA_DIST=ui-win.c
