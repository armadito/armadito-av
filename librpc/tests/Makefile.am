check_PROGRAMS= \
rpc-client-test

LDADD=$(top_builddir)/librpc/libarmadito-rpc.a @LIBJANSSON_LIBS@ -lm
AM_CPPFLAGS=-I$(top_srcdir)/librpc/include -I$(top_srcdir)/librpc -I$(top_srcdir)/libmodule/include @LIBJANSSON_CFLAGS@

#ipcsendtest_SOURCES=ipcsendtest.c
#ipcreceivetest_SOURCES=ipcreceivetest.c

rpc_client_test_SOURCES= rpc-client-test.c



