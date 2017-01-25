AUTOMAKE_OPTIONS=subdir-objects

noinst_LIBRARIES=libcore.a

libcore_a_SOURCES= \
action.c \
arch/linux/os/dir.c \
arch/linux/os/file.c \
arch/linux/os/mimetype.c \
arch/linux/builtin-modules/on-demand/ondemandmod.c \
arch/linux/builtin-modules/on-demand/ondemandmod.h \
armadito.c \
armadito_p.h \
conf.c \
confparser.c \
confparser.h \
error.c \
event.c \
info.c \
module.c \
module_p.h \
ondemand.c \
report.c \
report_p.h \
scanconf.c \
scanctx.c \
status.c \
status_p.h 

libcore_a_CFLAGS=-I$(top_srcdir) -I$(top_srcdir)/libmodule/include -I$(top_srcdir)/libcore/include -I$(top_srcdir)/libcore/arch/linux
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
include/core/file.h \
include/core/handle.h \
include/core/info.h \
include/core/io.h \
include/core/mimetype.h \
include/core/ondemand.h \
include/core/scanconf.h \
include/core/scanctx.h \
include/core/status.h
