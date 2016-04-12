AC_INIT(armadito-mod-clamav,1.5.3,francois@dechelle.net)
AM_INIT_AUTOMAKE

# collect arguments
# debug
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[enable debugging @<:@default is yes@:>@]),
	,
	enable_debug="yes")

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

PKG_PROG_PKG_CONFIG

# check for libarmadito
PKG_CHECK_MODULES(LIBARMADITO, libarmadito, [HAVE_LIBARMADITO=yes], [HAVE_LIBARMADITO=no])
if test "$HAVE_LIBARMADITO" = "no"; then
   AC_MSG_ERROR("cannot find ARMADITO library")
fi

# check for libclamav
PKG_CHECK_MODULES(LIBCLAMAV, libclamav, [HAVE_LIBCLAMAV=yes], [HAVE_LIBCLAMAV=no])
if test "$HAVE_LIBCLAMAV" = "no"; then
   AC_MSG_ERROR("cannot find ClamAV library")
fi

# check for glib
#PKG_CHECK_MODULES(GLIB2, glib-2.0, [HAVE_GLIB2=yes], [HAVE_GLIB2=no])
#if test "$HAVE_GLIB2" = "no"; then
#   AC_MSG_ERROR("cannot find glib-2.0 library")
#fi

# Output files
AC_CONFIG_FILES([
Makefile
src/Makefile
conf/Makefile
])
AC_OUTPUT
