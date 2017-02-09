AUTOMAKE_OPTIONS=subdir-objects

TESTS = rpc-client-test

check_PROGRAMS= \
rpc-client-test \
rpc-server-test

LDFLAGS=$(PTHREAD_CFLAGS) $(COVERAGE_LDFLAGS)
LDADD=$(top_builddir)/librpc/jrpc/libjrpc.a $(PTHREAD_LIBS) @LIBJANSSON_LIBS@ -lm
AM_CPPFLAGS=$(PTHREAD_CFLAGS) -I$(top_srcdir) -I$(top_srcdir)/librpc/jrpc/include -I$(top_srcdir)/libmodule/include @LIBJANSSON_CFLAGS@ -save-temps $(PTHREAD_LIBS) $(COVERAGE_CFLAGS)

COMMON_SOURCES= \
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
