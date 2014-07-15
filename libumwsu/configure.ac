m4_include([version.m4])
m4_define([LIBUMWSU_VERSION],VERSION_MAJOR[.]VERSION_MINOR[.]VERSION_PATCH)

AC_INIT(libumwsu,LIBUMWSU_VERSION,francois@dechelle.net)
AM_INIT_AUTOMAKE

# collect arguments
# debug
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[enable debugging @<:@default is yes@:>@]),
	,
	enable_debug="yes")

# configuration header
AM_CONFIG_HEADER(libumwsu-autoconf.h)

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

# Output files
AC_CONFIG_FILES([
Makefile
include/Makefile
include/libumwsu/Makefile
])
AC_OUTPUT
