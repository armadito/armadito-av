#SUBDIRS=conf

#check_PROGRAMS=testfind1 testuhuru1 testuhuruscan1 testxmlreport1 testprotocol1 testprotocol2
check_PROGRAMS=testuhuru1 testuhuruscan1 testxmlreport1 

AM_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src/lib @LIBXML2_CFLAGS@
LDADD=$(top_builddir)/src/lib/libuhuru.la @GMODULE2_LIBS@ @LIBXML2_LIBS@ -lmagic

#testfind1_SOURCES=testfind1.c
testuhuru1_SOURCES=testuhuru1.c
testuhuruscan1_SOURCES=testuhuruscan1.c
testxmlreport1_SOURCES=testxmlreport1.c
#testprotocol1_SOURCES=testprotocol1.c
#testprotocol2_SOURCES=testprotocol2.c

#modulesdir=$(pkglibdir)/modules
#modules_LTLIBRARIES=testmodule.la

#testmodule_la_SOURCES=testmodule.c
#testmodule_la_LDFLAGS= -module -avoid-version
#testmodule_la_CFLAGS=-I$(top_srcdir)/include
