check_PROGRAMS=ipcsendtest ipcreceivetest

LDADD=$(top_builddir)/libipc/libarmadito-ipc.a @LIBJANSSON_LIBS@
AM_CPPFLAGS=-I$(top_srcdir)/libipc/include -I$(top_srcdir)/libmodule/include @LIBJANSSON_CFLAGS@

ipcsendtest_SOURCES=ipcsendtest.c
ipcreceivetest_SOURCES=ipcreceivetest.c

