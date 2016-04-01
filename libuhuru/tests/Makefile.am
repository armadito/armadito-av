AUTOMAKE_OPTIONS=subdir-objects no-dependencies

#check_PROGRAMS=testfind1 testuhuru1 testuhuruscan1 testxmlreport1 testprotocol1 testprotocol2
check_PROGRAMS=testuhuru1 testuhuruscan1 testxmlreport1 testconfparser1 testipcreceive1 testipcsend1 testdir1 testjsonprint1 testconf1

AM_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src @GMODULE2_CFLAGS@ @LIBXML2_CFLAGS@
LDADD=$(top_builddir)/src/libcore/libuhuru_core.la @GMODULE2_LIBS@ @LIBXML2_LIBS@ -lmagic
testuhuru1_SOURCES=testuhuru1.c
testuhuruscan1_SOURCES=testuhuruscan1.c
testxmlreport1_SOURCES=testxmlreport1.c
testconfparser1_SOURCES=testconfparser1.c
testipcsend1_SOURCES=testipcsend1.c ../src/linux/daemon/ipc.c ../src/linux/daemon/ipc.h
testipcreceive1_SOURCES=testipcreceive1.c ../src/linux/daemon/ipc.c ../src/linux/daemon/ipc.h
testdir1_SOURCES=testdir1.c
testconf1_SOURCES=testconf1.c

testjsonprint1_SOURCES=testjsonprint1.c
testjsonprint1_CFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/include/config -I$(top_srcdir)/src -I$(top_srcdir)/src/json/ui @LIBJSONC_CFLAGS@
testjsonprint1_LDADD=$(top_builddir)/src/json/ui/libuhuru_json.la $(top_builddir)/src/libcore/libuhuru_core.la @LIBJSONC_LIBS@ -lmagic

#testfind1_SOURCES=testfind1.c
#testprotocol1_SOURCES=testprotocol1.c
#testprotocol2_SOURCES=testprotocol2.c

#modulesdir=$(pkglibdir)/modules
#modules_LTLIBRARIES=testmodule.la

#testmodule_la_SOURCES=testmodule.c
#testmodule_la_LDFLAGS= -module -avoid-version
#testmodule_la_CFLAGS=-I$(top_srcdir)/include

test_options:
	-./testopt -h
	-./testopt --help
	-./testopt --nopt
	-./testopt -n
	-./testopt --mopt
	-./testopt -m
	-./testopt --aarg=42
	-./testopt -a 42
	-./testopt --barg=43
	-./testopt -b 43
	-./testopt --foo
	-./testopt --foo=666
	-./testopt -f
	-./testopt -f 666
	-./testopt --nopt -a 42
	-./testopt -m --barg=43
