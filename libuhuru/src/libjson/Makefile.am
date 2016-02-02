# TESTING:
# nc -Ulk /tmp/.uhuru-ihm | ./libuhuru/tests/testjsonprint1 
# echo -en '{ "av_request":"scan", "id":123, "params": {"path":"/home/francois/Bureau"}}\r\n\r\n' | nc -U /tmp/.uhuru-daemon  | ./libuhuru/tests/testjsonprint1 

AUTOMAKE_OPTIONS=subdir-objects

lib_LTLIBRARIES=libuhuru_json.la

libuhuru_json_la_SOURCES= \
uhurujson.c \
uhurujson.h \
print.c \
print.h \
scan.c \
scan.h \
state.c \
state.h \
../linux/net/unixsockclient.c \
../linux/net/unixsockclient.h

libuhuru_json_la_CFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/include/config -I$(top_srcdir)/src -I$(top_srcdir)/src/linux
libuhuru_json_la_LIBADD= 

libuhuru_json_la_CFLAGS+= @LIBJSONC_CFLAGS@
libuhuru_json_la_LIBADD+= @LIBJSONC_LIBS@
