AC_INIT(libumwsu,1.2.0,francois@dechelle.net)
AM_INIT_AUTOMAKE

# collect arguments
# debug
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[enable debugging @<:@default is yes@:>@]),
	,
	enable_debug="yes")

# configuration header
AM_CONFIG_HEADER(libumwsu-config.h)

# This is needed when the first PKG_CHECK_MODULES call is inside a conditional (not the case here, but who knows)
PKG_PROG_PKG_CONFIG

# check for needed programs
AC_PROG_CC([gcc])
AC_PROG_LIBTOOL

# check for debug
AC_MSG_CHECKING(for debug)
AC_MSG_RESULT($enable_debug)
AM_CONDITIONAL([DEBUG], [test "$enable_debug" = "yes"])
CFLAGS=
if test "$enable_debug" = "yes"; then
   AC_DEFINE(DEBUG,1,[Define DEBUG to enable debug messages])
   CFLAGS="${CFLAGS} -g"
fi

# check for gmodule
PKG_CHECK_MODULES(GMODULE2, gmodule-2.0, [HAVE_GMODULE2=yes], [HAVE_GMODULE2=no])
if test "$HAVE_GMODULE2" = "no"; then
   AC_MSG_ERROR("cannot find gmodule2 library")
fi

# check for libxml2
PKG_CHECK_MODULES(LIBXML2, libxml-2.0, [HAVE_LIBXML2=yes], [HAVE_LIBXML2=no])
if test "$HAVE_LIBXML2" = "no"; then
   AC_MSG_ERROR("cannot find libxml2 library")
fi

# check for libcurl
#PKG_CHECK_MODULES(LIBCURL, libcurl, [HAVE_LIBCURL=yes], [HAVE_LIBCURL=no])
#if test "$HAVE_LIBCURL" = "no"; then
#   AC_MSG_ERROR("cannot find libcurl library")
#fi

# check for libmagic, that has no pkg-config
AC_CHECK_LIB(magic, magic_open, [HAVE_LIBMAGIC=yes], [HAVE_LIBMAGIC=no])
if test "$HAVE_LIBMAGIC" = "no"; then
   AC_MSG_ERROR("cannot find -lmagic")
fi
AC_CHECK_HEADER(magic.h, [HAVE_MAGIC_H=yes], [HAVE_MAGIC_H=no])
if test "$HAVE_MAGIC_H" = "no"; then
   AC_MSG_ERROR("cannot find magic.h")
fi

# documentation
# check for asciidoc and a2x
AC_PATH_PROG(ASCIIDOC, asciidoc)
AC_PATH_PROG(A2X, a2x)
AM_CONDITIONAL([COND_DOC], [test ! -z "$ASCIIDOC" -a ! -z "$A2X" ])

# Output files
AC_CONFIG_FILES([
Makefile
conf/Makefile
doc/Makefile
doc/man/Makefile
doc/man/fr/Makefile
include/Makefile
include/libumwsu/Makefile
pkgconfig/Makefile
src/Makefile
src/lib/Makefile
src/daemon/Makefile
tests/Makefile
tools/Makefile
])
AC_OUTPUT
