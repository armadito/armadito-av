AC_INIT(armadito-gui,0.10.1,fdechelle@teclib.com)
AM_INIT_AUTOMAKE

# collect arguments
AC_ARG_WITH([nwjs],
	AS_HELP_STRING([--with-nwjs=TARBALL],[specify path to nw.js tarball]),
	with_nwjs="$withval",
	with_nwjs="none")

# checks for programs.
AC_PATH_PROG(ZIP, zip)
AC_PATH_PROG(NPM, npm)

# check for nw.js
AC_MSG_CHECKING(for nw.js tarball)
if test "$with_nwjs" != "none" ; then
   NWJS_TARBALL="$with_nwjs"
else
   NWJS_TARBALL=`find $top_srcdir/nwjs -name 'nwjs-v*.tar.gz'`
fi
AC_MSG_RESULT($NWJS_TARBALL)
if test ! -f "$NWJS_TARBALL" ; then
   AC_MSG_ERROR("cannot find $NWJS_TARBALL")
fi
AC_SUBST(NWJS_TARBALL)

# make symbolic links
AC_CONFIG_LINKS([bower.json:bower.json package.json:package.json])

# Output files
AC_CONFIG_FILES([
linux/Makefile
Makefile
])
AC_OUTPUT
