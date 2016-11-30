check_PROGRAMS= \
rpc-notify-send

LDADD=$(top_builddir)/librpc/libarmadito-rpc.a @LIBJANSSON_LIBS@
AM_CPPFLAGS=-I$(top_srcdir)/librpc/include -I$(top_srcdir)/libmodule/include @LIBJANSSON_CFLAGS@

#ipcsendtest_SOURCES=ipcsendtest.c
#ipcreceivetest_SOURCES=ipcreceivetest.c

rpc_notify_send_SOURCES= rpc-notify-send.c



