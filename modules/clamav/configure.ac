AC_INIT(uhuru-mod-clamav,1.2.0,francois@dechelle.net)
AM_INIT_AUTOMAKE

# collect arguments
# debug
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[enable debugging @<:@default is yes@:>@]),
	,
	enable_debug="yes")
# libuhuru
AC_ARG_WITH([libuhuru],
	AS_HELP_STRING([--with-libuhuru=DIR],[specify libuhuru install dir @<:@default is /usr@:>@]),
	,
	with_libuhuru="$withval")

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

# check for libuhuru
if test x"$with_libuhuru" != x ; then
   export PKG_CONFIG_PATH="$with_libuhuru/lib/pkgconfig"
fi
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(LIBUHURU, libuhuru, [HAVE_LIBUHURU=yes], [HAVE_LIBUHURU=no])
if test "$HAVE_LIBUHURU" = "no"; then
   AC_MSG_ERROR("cannot find UHURU library")
fi
#LIBUHURU_MODDIR=$(pkg-config --variable=moduledir libuhuru)
#AC_SUBST(LIBUHURU_MODDIR)

# check for libclamav
PKG_CHECK_MODULES(LIBCLAMAV, libclamav, [HAVE_LIBCLAMAV=yes], [HAVE_LIBCLAMAV=no])
if test "$HAVE_LIBCLAMAV" = "no"; then
   AC_MSG_ERROR("cannot find ClamAV library")
fi

# Output files
AC_CONFIG_FILES([
Makefile
src/Makefile
conf/Makefile
])
AC_OUTPUT
