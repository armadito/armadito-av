check_PROGRAMS= \
rpc-client-test \
rpc-server-test

LDADD=$(top_builddir)/libjrpc/libjrpc.a @LIBJANSSON_LIBS@ -lm
AM_CPPFLAGS=-I$(top_srcdir)/libjrpc/include -I$(top_srcdir)/libmodule/include @LIBJANSSON_CFLAGS@ -save-temps

#ipcsendtest_SOURCES=ipcsendtest.c
#ipcreceivetest_SOURCES=ipcreceivetest.c

rpc_client_test_SOURCES= rpc-client-test.c libtest.c libtest.h test.h test.c test-marshall.h
rpc_server_test_SOURCES= rpc-server-test.c libtest.c libtest.h test.h test.c test-marshall.h
