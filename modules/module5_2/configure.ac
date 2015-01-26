AC_INIT(uhuru-mod-module5-2,1.0.0,francois@dechelle.net)
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
	with_libumwsu="$withval",
	)
# database directory
AC_ARG_WITH([dbdir],
	AS_HELP_STRING([--with-dbdir=DIR],[specify path to databases @<:@default is /var/lib/umwsu/module5-2/db@:>@]),
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

# check for libumwsu
if test x"$with_libumwsu" != x ; then
   export PKG_CONFIG_PATH="$with_libumwsu/lib/pkgconfig"
fi
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(LIBUMWSU, libumwsu, [HAVE_LIBUMWSU=yes], [HAVE_LIBUMWSU=no])
if test "$HAVE_LIBUMWSU" = "no"; then
   AC_MSG_ERROR("cannot find UMWSU library")
fi
#LIBUMWSU_MODDIR=$(pkg-config --variable=moduledir libumwsu)
#AC_SUBST(LIBUMWSU_MODDIR)

# database directory
AC_MSG_CHECKING(for databases directory)
if test "$with_dbdir" = "no" ; then
   MODULE5_2_DBDIR="$localstatedir/lib/umwsu/module5-2/db"
else
   MODULE5_2_DBDIR="$with_dbdir"
fi
AC_MSG_RESULT($MODULE5_2_DBDIR)
if test ! -f "$MODULE5_2_DBDIR/database.elfdata" ; then
   AC_MSG_WARN("cannot find $MODULE5_2_DBDIR/database.elfdata, continuing anyway")
fi
AC_SUBST(MODULE5_2_DBDIR)

# Output files
AC_CONFIG_FILES([
Makefile
src/Makefile
])
AC_OUTPUT
