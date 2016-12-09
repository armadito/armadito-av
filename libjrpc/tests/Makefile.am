check_PROGRAMS= \
rpc-client-test \
rpc-server-test

LDADD=$(top_builddir)/libjrpc/libjrpc.a @LIBJANSSON_LIBS@ -lm
AM_CPPFLAGS=-I$(top_srcdir)/libjrpc/include -I$(top_srcdir)/libjrpc -I$(top_srcdir)/libmodule/include @LIBJANSSON_CFLAGS@

#ipcsendtest_SOURCES=ipcsendtest.c
#ipcreceivetest_SOURCES=ipcreceivetest.c

rpc_client_test_SOURCES= rpc-client-test.c libtest.c libtest.h
rpc_server_test_SOURCES= rpc-server-test.c libtest.c libtest.h
