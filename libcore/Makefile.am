AUTOMAKE_OPTIONS=subdir-objects

noinst_LIBRARIES=libcore.a

libcore_a_SOURCES= \
action.c \
arch/linux/dir.c \
arch/linux/file.c \
arch/linux/mimetype.c \
armadito.c \
armadito_p.h \
conf.c \
confparser.c \
confparser.h \
error.c \
filectx.c \
info.c \
module.c \
module_p.h \
ondemand.c \
ondemand_p.h \
report.c \
report_p.h \
scan.c \
scanconf.c \
scan_p.h \
status.c \
status_p.h 

libcore_a_CFLAGS=-I$(top_srcdir) -I$(top_srcdir)/libmodule/include -I$(top_srcdir)/libcore/include
#libcore_a_LIBADD= 

libcore_a_CFLAGS+= @GLIB2_CFLAGS@
#libcore_a_LIBADD+= @GLIB2_LIBS@

libcore_a_CFLAGS+= @GMODULE2_CFLAGS@
#libcore_a_LIBADD+= @GMODULE2_LIBS@

libcore_a_CFLAGS+= @GTHREAD2_CFLAGS@
#libcore_a_LIBADD+= @GTHREAD2_LIBS@

install-exec-hook:
# these 2 lines were for alerts directory
#	-mkdir -p $(DESTDIR)$(localstatedir)/spool/armadito
#	-chmod 0777 $(DESTDIR)$(localstatedir)/spool/armadito
	-mkdir -p $(DESTDIR)$(localstatedir)/lib/armadito/tmp


noinst_HEADERS= \
include/core/action.h \
include/core/conf.h \
include/core/dir.h \
include/core/error.h \
include/core/event.h \
include/core/filectx.h \
include/core/file.h \
include/core/handle.h \
include/core/io.h \
include/core/mimetype.h \
include/core/ondemand.h \
include/core/scanconf.h \
include/core/scan.h
