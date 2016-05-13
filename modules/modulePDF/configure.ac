AC_INIT(armadito-mod-modulePDF, [0.9.0], ufausther@teclib.com)
AM_INIT_AUTOMAKE

# collect arguments
# debug
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[enable debugging @<:@default is yes@:>@]),
	,
	enable_debug="yes")

# check for analysis module sources
AC_MSG_CHECKING(for armaditopdf sources)
AC_CONFIG_SRCDIR(armaditopdf/module7/main.c)
AC_MSG_RESULT(yes)

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

# Output files
AC_CONFIG_FILES([
Makefile
])
AC_OUTPUT
