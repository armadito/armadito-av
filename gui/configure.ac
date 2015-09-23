AC_INIT([uhuru-qt], [1.4.0], [f.dechelle@uhuru-solutions.com])
AM_INIT_AUTOMAKE

# debug
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[enable debugging @<:@default is yes@:>@]),
	,
	enable_debug="yes")

# configuration header
AM_CONFIG_HEADER(uhuru-qt-config.h)

CFLAGS=-g

# check for debug
AC_MSG_CHECKING(for debug)
AC_MSG_RESULT($enable_debug)
AM_CONDITIONAL([DEBUG], [test "$enable_debug" = "yes"])
CFLAGS=
if test "$enable_debug" = "yes"; then
   AC_DEFINE(DEBUG,1,[Define DEBUG to enable debug messages])
   CFLAGS="${CFLAGS} -g"
fi

# Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC

# check for libuhuru
PKG_CHECK_MODULES(LIBUHURU, libuhuru, [HAVE_LIBUHURU=yes], [HAVE_LIBUHURU=no])
if test "$HAVE_LIBUHURU" = "no"; then
   AC_MSG_ERROR("cannot find UHURU library")
fi

# phasing out GTK
#PKG_CHECK_MODULES(GTK, gtk+-2.0, HAVE_GTK2="yes", HAVE_GTK2="no")
HAVE_GTK2="no"
if test "$HAVE_GTK2" = "yes" ; then
#   PKG_CHECK_MODULES(INDICATOR, indicator-0.4)
   PKG_CHECK_MODULES(APPINDICATOR, appindicator-0.1, HAVE_APPINDICATOR="yes", HAVE_APPINDICATOR="no")
   if test "$HAVE_APPINDICATOR" = "yes"; then
      AC_DEFINE(HAVE_APPINDICATOR, 1, [Define to 1 if you have the appindicator library])
   fi   

   PKG_CHECK_MODULES(GOBJECT, glib-2.0 gobject-2.0)
   PKG_CHECK_MODULES(GIO, gio-2.0 gio-unix-2.0)

   # check for libnotify
   PKG_CHECK_MODULES(LIBNOTIFY, libnotify)

   # i18n
   #IT_PROG_INTLTOOL([0.35.0])
   #GETTEXT_PACKAGE=uhuru-linux-gui
   #AC_SUBST(GETTEXT_PACKAGE)
   #AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])
   #AM_GLIB_GNU_GETTEXT
fi
AM_CONDITIONAL(COND_GTK_GUI, test x"$HAVE_GTK2" = "xyes" -o x"$HAVE_GTK3" = "xyes")

# check for Qt >= 4
PKG_CHECK_MODULES(QT, QtCore QtGui >= 4.2.0 QtSvg QtDBus, HAVE_QT="yes", HAVE_QT="no")
if test "$HAVE_QT" = "no" ; then
   PKG_CHECK_MODULES(QT, Qt5Core Qt5Gui Qt5Svg Qt5DBus, HAVE_QT5="yes", HAVE_QT5="no")
   if test "$HAVE_QT5" = "yes" ; then HAVE_QT="yes" ; else HAVE_QT="no" ; fi
fi
	
if test "$HAVE_QT" = "yes" ; then
   AC_PROG_CXX

   # check for moc, uic and rcc
   AC_PATH_PROGS(MOC, moc-qt4 moc moc-qt5, no, [$PATH:$QTDIR/bin])
   AC_PATH_PROGS(UIC, uic-qt4 uic uic-qt5, no, [$PATH:$QTDIR/bin])
   AC_PATH_PROGS(RCC, rcc rcc-qt5, no, [$PATH:$QTDIR/bin])
   AC_PATH_PROGS(LUPDATE, lupdate-qt4 lupdate, no, [$PATH:$QTDIR/bin])
   AC_PATH_PROGS(LRELEASE, lrelease-qt4 lrelease, no, [$PATH:$QTDIR/bin])
   if test "x$MOC" = "xno" -o "x$UIC" = "xno" -o "x$RCC" = "xno"  -o "x$LUPDATE" = "xno"  -o "x$LRELEASE" = "xno" ; then
      AC_MSG_WARN([cannot find 'moc', 'uic', 'rcc', 'lupdate' or 'lrelease', please install the Qt development tools package.])
      HAVE_QT="no"
   fi
fi
AM_CONDITIONAL(COND_QT_GUI, test x"$HAVE_QT" = "xyes")

# output files
AC_CONFIG_FILES([
Makefile
qt/Makefile
qt/icons/Makefile
qt/translations/Makefile
])
AC_OUTPUT
