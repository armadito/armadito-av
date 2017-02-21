SUBDIRS= jrpc

AUTOMAKE_OPTIONS=subdir-objects

noinst_LIBRARIES=librpc.a

librpc_a_SOURCES= \
rpctypes.c \
rpcbe.c \
unixio.c

librpc_a_CFLAGS=-I$(top_srcdir) -I$(top_srcdir)/libmodule/include -I$(top_srcdir)/libcore/include -I$(top_srcdir)/librpc/include -I$(top_srcdir)/librpc/jrpc/include @GLIB2_CFLAGS@ @GIO2_CFLAGS@ @GTHREAD2_CFLAGS@ @LIBJANSSON_CFLAGS@

noinst_HEADERS= \
include/rpc/rpcbe.h \
include/rpc/rpcdefs.h \
include/rpc/rpctypes.h

noinst_DATA=rpcschema.json

JSON_CPP=$(CPP) -E
JSON_CPP_FLAGS=-DGENERATE_JSON_SCHEMA -P -I$(top_srcdir)/librpc/include -I$(top_srcdir)/librpc/jrpc/include

rpcschema.json: $(srcdir)/include/rpc/rpcschema.h
	$(JSON_CPP) $(JSON_CPP_FLAGS) $< | sed -e '/^$$/d' -e 's/" "//' > $@ 

rpcschema.json: $(srcdir)/include/rpc/rpcdefs.h $(srcdir)/jrpc/include/libjrpc/jsonschema.h

CLEANFILES= rpcschema.json
