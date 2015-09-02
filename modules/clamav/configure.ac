AC_INIT(uhuru-mod-clamav,1.4.0,francois@dechelle.net)
AM_INIT_AUTOMAKE

# collect arguments
# debug
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[enable debugging @<:@default is yes@:>@]),
	,
	enable_debug="yes")

# database directory
AC_ARG_WITH([dbdir],
	AS_HELP_STRING([--with-dbdir=DIR],[specify path to databases @<:@default is /var/lib/uhuru/bases/clamav@:>@]),
	with_dbdir="$withval",
	with_dbdir="no")

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

# check for libclamav
PKG_CHECK_MODULES(LIBCLAMAV, libclamav, [HAVE_LIBCLAMAV=yes], [HAVE_LIBCLAMAV=no])
if test "$HAVE_LIBCLAMAV" = "no"; then
   AC_MSG_ERROR("cannot find ClamAV library")
fi

# check for glib
PKG_CHECK_MODULES(GLIB2, glib-2.0, [HAVE_GLIB2=yes], [HAVE_GLIB2=no])
if test "$HAVE_GLIB2" = "no"; then
   AC_MSG_ERROR("cannot find glib-2.0 library")
fi

# database directory
AC_MSG_CHECKING(for databases directory)
if test "$with_dbdir" = "no" ; then
   MODULE_CLAMAV_DBDIR="$localstatedir/lib/uhuru/bases/clamav"
else
   MODULE_CLAMAV_DBDIR="$with_dbdir"
fi
AC_MSG_RESULT($MODULE_CLAMAV_DBDIR)
if test ! -f "$MODULE_CLAMAV_DBDIR/daily.cld" ; then
   AC_MSG_WARN("cannot find $MODULE_CLAMAV_DBDIR/daily.cld, continuing anyway")
fi
AC_SUBST(MODULE_CLAMAV_DBDIR)

# Output files
AC_CONFIG_FILES([
Makefile
src/Makefile
conf/Makefile
])
AC_OUTPUT
