AC_INIT([clock-gui], [1.0.0], [francois@dechelle.net])
AM_INIT_AUTOMAKE

CFLAGS=-g

AC_PROG_CC

# check for GTK
PKG_CHECK_MODULES(GTK, gtk+-2.0)
PKG_CHECK_MODULES(GOBJECT, glib-2.0 gobject-2.0)

# output files
AC_CONFIG_FILES([
Makefile
src/Makefile
])
AC_OUTPUT
