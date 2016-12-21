AUTOMAKE_OPTIONS=subdir-objects

check_PROGRAMS= \
rpc-client-test \
rpc-server-test

LDFLAGS=$(PTHREAD_CFLAGS)
LDADD=$(top_builddir)/libjrpc/libjrpc.a $(PTHREAD_LIBS) @LIBJANSSON_LIBS@ -lm
AM_CPPFLAGS=$(PTHREAD_CFLAGS) -I$(top_srcdir)/libjrpc/include -I$(top_srcdir)/libmodule/include @LIBJANSSON_CFLAGS@ -save-temps $(PTHREAD_LIBS)

#ipcsendtest_SOURCES=ipcsendtest.c
#ipcreceivetest_SOURCES=ipcreceivetest.c

COMMON_SOURCES= \
armadito.c \
armadito-marshall.h \
test.h \
test.c \
test-marshall.h \
unix.c \
unix.h


rpc_client_test_SOURCES= \
rpc-client-test.c \
$(COMMON_SOURCES)

rpc_server_test_SOURCES= \
rpc-server-test.c \
$(COMMON_SOURCES)
