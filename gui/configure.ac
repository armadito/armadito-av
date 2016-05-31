AC_INIT(armadito-gui,0.10.1,fdechelle@teclib.com)
AM_INIT_AUTOMAKE

# collect arguments
AC_ARG_WITH([nwjs],
	AS_HELP_STRING([--with-nwjs=TARBALL],[specify path to nw.js tarball]),
	with_nwjs="$withval",
	with_nwjs="none")

# checks for programs.
AC_PATH_PROG(NPM, npm)
if test x'$NPM' = x ; then
   AC_MSG_ERROR("cannot find npm")
fi

# check for nw.js
AC_MSG_CHECKING(for nw.js tarball)
NWJS_TARBALL='none'
if test "$with_nwjs" != "none" ; then
   NWJS_TARBALL="$with_nwjs"
else
   FOUND_NWJS_TARBALL=`find $srcdir -name 'nwjs-v*.tar.gz'`
   if test x"$FOUND_NWJS_TARBALL" != x ; then
   NWJS_TARBALL=$FOUND_NWJS_TARBALL
   fi
fi
AC_MSG_RESULT($NWJS_TARBALL)
if test ! -f "$NWJS_TARBALL" ; then
   AC_MSG_ERROR("cannot find NW.JS tarball")
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
