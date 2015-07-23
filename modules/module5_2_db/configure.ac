AC_INIT(uhuru-mod-module5-2-db,1.3.0,francois@dechelle.net)
AM_INIT_AUTOMAKE(dist-bzip2 no-dist-gzip)

# collect arguments
# database directory
AC_ARG_WITH([dbdir],
	AS_HELP_STRING([--with-dbdir=DIR],[specify path to databases @<:@default is /var/lib/umwsu/module5-2/db@:>@]),
	with_dbdir="$withval",
	with_dbdir="no")

# databases
AC_MSG_CHECKING(for databases)
if test "$with_dbdir" = "no" ; then
   if test -f "$srcdir/db/linux/database.elfdata" ; then
      MODULE5_2_DBDIR="$srcdir/db"
   else
      MODULE5_2_DBDIR="$localstatedir/lib/umwsu/module5-2/db"
   fi
else
   MODULE5_2_DBDIR="$with_dbdir"
fi
AC_MSG_RESULT($MODULE5_2_DBDIR)
if test ! -f "$MODULE5_2_DBDIR/linux/database.elfdata" ; then
   AC_MSG_WARN("cannot find $MODULE5_2_DBDIR/linux/database.elfdata, continuing anyway")
fi
AC_SUBST(MODULE5_2_DBDIR)

# Output files
AC_CONFIG_FILES([
Makefile
])
AC_OUTPUT
