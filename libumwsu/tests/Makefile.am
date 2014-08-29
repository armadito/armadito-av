check_PROGRAMS=testfind1 testumwsu1 testumwsuscan1 testxmlreport1

testfind1_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src
testfind1_LDADD=$(top_builddir)/src/libumwsu.la @GMODULE2_LIBS@ -lmagic
testfind1_SOURCES=testfind1.c

testumwsu1_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src
testumwsu1_LDADD=$(top_builddir)/src/libumwsu.la @GMODULE2_LIBS@ -lmagic
testumwsu1_SOURCES=testumwsu1.c

testumwsuscan1_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src
testumwsuscan1_LDADD=$(top_builddir)/src/libumwsu.la @GMODULE2_LIBS@ -lmagic
testumwsuscan1_SOURCES=testumwsuscan1.c

modulesdir=$(pkglibdir)/modules
modules_LTLIBRARIES=testmodule.la

testmodule_la_SOURCES=testmodule.c
testmodule_la_LDFLAGS= -module -avoid-version
testmodule_la_CFLAGS=-I$(top_srcdir)/include

testxmlreport1_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src @LIBXML2_CFLAGS@
testxmlreport1_LDADD=$(top_builddir)/src/libumwsu.la @GMODULE2_LIBS@ @LIBXML2_LIBS@ -lmagic
testxmlreport1_SOURCES=testxmlreport1.c
