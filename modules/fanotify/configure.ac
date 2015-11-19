AC_INIT(uhuru-mod-fanotify, [1.5.2],francois@dechelle.net)
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

# check for libuhuru
PKG_CHECK_MODULES(LIBUHURU, libuhuru, [HAVE_LIBUHURU=yes], [HAVE_LIBUHURU=no])
if test "$HAVE_LIBUHURU" = "no"; then
   AC_MSG_ERROR("cannot find UHURU library")
fi

# check for glib
PKG_CHECK_MODULES(GLIB2, glib-2.0, [HAVE_GLIB2=yes], [HAVE_GLIB2=no])
if test "$HAVE_GLIB2" = "no"; then
   AC_MSG_ERROR("cannot find glib-2.0 library")
fi
PKG_CHECK_MODULES(GIO2, gio-2.0 gio-unix-2.0, [HAVE_GIO2=yes], [HAVE_GIO2=no])
if test "$HAVE_GIO2" = "no"; then
   AC_MSG_ERROR("cannot find gio-2.0 library")
fi

# check for fanotify
AC_CHECK_FUNCS(fanotify_init, [HAVE_FANOTIFY_INIT=1], [HAVE_FANOTIFY_INIT=0])
if test "$HAVE_FANOTIFY_INIT" = "0" ; then
   AC_MSG_ERROR("fanotify is not availble on this Linux distribution")
fi

# Output files
AC_CONFIG_FILES([
Makefile
src/Makefile
conf/Makefile
])
AC_OUTPUT
