check_PROGRAMS=testfind1 testumwsu1

#testmodule1_CFLAGS= -I$(top_srcdir)/include
#testmodule1_LDADD=$(top_builddir)/src/libumwsu.la @GMODULE2_LIBS@
#testmodule1_SOURCES=testmodule1.c

modulesdir=$(pkglibdir)/modules
modules_LTLIBRARIES=module1.la

module1_la_SOURCES=module1.c
module1_la_LDFLAGS= -module -avoid-version

testfind1_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src
testfind1_LDADD=$(top_builddir)/src/libumwsu.la @GMODULE2_LIBS@ -lmagic
testfind1_SOURCES=testfind1.c

testumwsu1_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src
testumwsu1_LDADD=$(top_builddir)/src/libumwsu.la @GMODULE2_LIBS@ -lmagic
testumwsu1_SOURCES=testumwsu1.c
