AC_INIT(libuhuru,1.4.1,francois@dechelle.net)
AM_INIT_AUTOMAKE

# collect arguments
# debug
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[enable debugging @<:@default is yes@:>@]),
	,
	enable_debug="yes")

# configuration header
AM_CONFIG_HEADER(libuhuru-config.h)

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

# check for glib stuff
PKG_CHECK_MODULES(GMODULE2, gmodule-2.0 gthread-2.0, [HAVE_GMODULE2=yes], [HAVE_GMODULE2=no])
if test "$HAVE_GMODULE2" = "no"; then
   AC_MSG_ERROR("cannot find gmodule2 library")
fi
PKG_CHECK_MODULES(GIO2, gio-2.0 gio-unix-2.0, [HAVE_GIO2=yes], [HAVE_GIO2=no])
if test "$HAVE_GIO2" = "no"; then
   AC_MSG_ERROR("cannot find glib gio library")
fi
# check for g_thread_new/g_thread_create
SAVE_CFLAGS=${CFLAGS}
CFLAGS="${GMODULE2_CFLAGS} ${CFLAGS}"
SAVE_LDFLAGS=${LDFLAGS}
LDFLAGS="${GMODULE2_LIBS} ${LDFLAGS}"
AC_CHECK_LIB([glib-2.0], [g_thread_new],
		      [AC_DEFINE([HAVE_GTHREAD_NEW], [], [Define this if glib has the 'g_thread_new' function])])
AC_CHECK_LIB([glib-2.0], [g_thread_create],
		      [AC_DEFINE([HAVE_GTHREAD_CREATE], [], [Define this if glib has the 'g_thread_create' function])])
CFLAGS=${SAVE_CFLAGS}
unset SAVE_CFLAGS
LDFLAGS=${SAVE_LDFLAGS}
unset SAVE_LDFLAGS
# for now, disable deprecations, will need more tests in configure.ac
AC_DEFINE([GLIB_DISABLE_DEPRECATION_WARNINGS], [1], [Disable glib deprecation warnings])

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
include/libuhuru/Makefile
pkgconfig/Makefile
src/Makefile
src/lib/Makefile
src/scand/Makefile
src/watchd/Makefile
tests/Makefile
tools/Makefile
])
AC_OUTPUT
