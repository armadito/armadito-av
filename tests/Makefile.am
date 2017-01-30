SUBDIRS=web

AUTOMAKE_OPTIONS=subdir-objects no-dependencies

#check_PROGRAMS=testarmadito1 testarmaditoscan1 testxmlreport1 testconfparser1 testipcreceive1 testipcsend1 testdir1 testjsonprint1 testconf1

AM_CFLAGS=-I$(top_srcdir)/libmodule/include -I$(top_srcdir)/libcore/include -I$(top_srcdir) @GMODULE2_CFLAGS@ @LIBXML2_CFLAGS@
LDADD=$(top_builddir)/libarmadito/src/libarmadito.la @GMODULE2_LIBS@ @LIBXML2_LIBS@ -lmagic
#testarmadito1_SOURCES=testarmadito1.c
#testarmaditoscan1_SOURCES=testarmaditoscan1.c
#testxmlreport1_SOURCES=testxmlreport1.c
#testconfparser1_SOURCES=testconfparser1.c
#testconfparser1_CFLAGS=$(AM_CFLAGS) -I$(top_srcdir)/libarmadito/src 
#testipcsend1_SOURCES=testipcsend1.c ../linux/daemon/ipc.c ../linux/daemon/ipc.h
#testipcsend1_CFLAGS=$(AM_CFLAGS) -I$(top_srcdir)/linux
#testipcreceive1_SOURCES=testipcreceive1.c ../linux/daemon/ipc.c ../linux/daemon/ipc.h
#testipcreceive1_CFLAGS=$(AM_CFLAGS) -I$(top_srcdir)/linux
#testdir1_SOURCES=testdir1.c
#testconf1_SOURCES=testconf1.c

#testjsonprint1_SOURCES=testjsonprint1.c
#testjsonprint1_CFLAGS= -I$(top_srcdir)/libarmadito/include -I$(top_srcdir) -I$(top_srcdir)/linux -I$(top_srcdir)/json/ui @LIBJSONC_CFLAGS@
#testjsonprint1_LDADD=$(top_builddir)/json/ui/libarmadito_json.la $(top_builddir)/libarmadito/src/libarmadito.la @LIBJSONC_LIBS@ -lmagic
