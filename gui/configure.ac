AC_INIT(armadito-gui,0.10.1,fdechelle@teclib.com)
AM_INIT_AUTOMAKE

# collect arguments
AC_ARG_WITH([nwjs],
	AS_HELP_STRING([--with-nwjs=DIR],[specify path to directory containing nw.js tarball]),
	with_nwjs="$withval",
	with_nwjs="none")

# checks for programs.
AC_PATH_PROG(NPM, npm)
if test ! -x "$NPM" ; then
   AC_MSG_ERROR("cannot find npm")
fi

# check for nw.js
AC_MSG_CHECKING(for nw.js tarballs)
NWJS_TARBALL_DIR='none'
if test "$with_nwjs" != "none" ; then
   NWJS_TARBALL_DIR="$with_nwjs"
else
   NWJS_TARBALL_DIR=$srcdir
fi
NWJS_TARBALL_32=`find $NWJS_TARBALL_DIR -name 'nwjs-v*-linux-ia32.tar.gz'`
NWJS_TARBALL_64=`find $NWJS_TARBALL_DIR -name 'nwjs-v*-linux-x64.tar.gz'`
if test -f "$NWJS_TARBALL_32" -a -f "$NWJS_TARBALL_64" ; then
   AC_MSG_RESULT($NWJS_TARBALL_32 $NWJS_TARBALL_64)
else
   AC_MSG_RESULT("none found")
   AC_MSG_ERROR("cannot find NW.JS tarballs")
fi
AC_CANONICAL_HOST
case "$host_cpu" in
i*86)
	NWJS_TARBALL=$NWJS_TARBALL_32
	;;
x86_64)
	NWJS_TARBALL=$NWJS_TARBALL_64
	;;
esac
AC_SUBST(NWJS_TARBALL_32)
AC_SUBST(NWJS_TARBALL_64)
AC_SUBST(NWJS_TARBALL)

# make symbolic links
AC_CONFIG_LINKS([bower.json:bower.json package.json:package.json])

# Output files
AC_CONFIG_FILES([
linux/Makefile
Makefile
])
AC_OUTPUT
