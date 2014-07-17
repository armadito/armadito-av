check_PROGRAMS=testmodule1

testmodule1_CFLAGS= -I$(top_srcdir)/include
testmodule1_LDADD=$(top_builddir)/src/libumwsu.la @GMODULE2_LIBS@
testmodule1_SOURCES=testmodule1.c

lib_LTLIBRARIES=module1.la

module1_la_SOURCES=module1.c
module1_la_LDFLAGS= -module -avoid-version
