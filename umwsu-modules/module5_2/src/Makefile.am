modulesdir=@LIBUMWSU_MODDIR@
modules_LTLIBRARIES=module5_2.la

module5_2_la_SOURCES= \
module5_2.c \
org/databases.c \
org/databases.h \
org/distances.c \
org/distances.h \
org/elfReader.c \
org/elfReader.h \
org/errors.h \
org/kNN.c \
org/kNN.h \
org/main.c \
org/miniz.c \
org/miniz.h \
org/model.c \
org/model.h \
org/table.c \
org/table.h \
org/utils.h \
org/vector.c \
org/vector.h \
org/windowsTypes.h 

AM_CPPFLAGS= -DMODULE_52_DBDIR='"$(abs_top_srcdir)/src/org"'
module5_2_la_LDFLAGS= -module -avoid-version
module5_2_la_CFLAGS= -I$(srcdir)/org
module5_2_la_LIBADD=
module5_2_la_CFLAGS+= @LIBUMWSU_CFLAGS@
module5_2_la_LIBADD+= @LIBUMWSU_LIBS@

