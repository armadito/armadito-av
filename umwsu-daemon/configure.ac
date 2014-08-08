AC_INIT(umwsu-daemon,1.0.1,francois@dechelle.net)
AM_INIT_AUTOMAKE

# collect arguments
# debug
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[enable debugging @<:@default is yes@:>@]),
	,
	enable_debug="yes")
# libumwsu
AC_ARG_WITH([libumwsu],
	AS_HELP_STRING([--with-libumwsu=DIR],[specify libumwsu install dir @<:@default is /usr@:>@]),
	,
	with_libumwsu="$withval")

# check for needed programs
AC_PROG_CC([gcc])
AC_PROG_LIBTOOL

# start with clean CFLAGS
CFLAGS=

# check for debug
AC_MSG_CHECKING(for debug)
AC_MSG_RESULT($enable_debug)
AM_CONDITIONAL([DEBUG], [test "$enable_debug" = "yes"])
if test "$enable_debug" = "yes"; then
   AC_DEFINE(DEBUG,1,[Define DEBUG to enable debug messages])
   CFLAGS="${CFLAGS} -g"
fi

# check for libumwsu
if test x"$with_libumwsu" != x ; then
   export PKG_CONFIG_PATH="$with_libumwsu/lib/pkgconfig"
fi
#PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(LIBUMWSU, libumwsu, [HAVE_LIBUMWSU=yes], [HAVE_LIBUMWSU=no])
if test "$HAVE_LIBUMWSU" = "no"; then
   AC_MSG_ERROR("cannot find UMWSU library")
fi

# check for glib
PKG_CHECK_MODULES(GLIB2, glib-2.0, [HAVE_GLIB2=yes], [HAVE_GLIB2=no])
if test "$HAVE_GLIB2" = "no"; then
   AC_MSG_ERROR("cannot find glib2 library")
fi

# documentation
# check for asciidoc and a2x
AC_PATH_PROG(ASCIIDOC, asciidoc)
AC_PATH_PROG(A2X, a2x)
AM_CONDITIONAL([COND_DOC], [test ! -z "$ASCIIDOC" -a ! -z "$A2X" ])

# Output files
AC_CONFIG_FILES([
Makefile
src/Makefile
doc/Makefile
doc/man/Makefile
doc/man/fr/Makefile
])
AC_OUTPUT
