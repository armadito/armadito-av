AC_INIT(armadito-core,0.12.0-exp,fdechelle@teclib.com)
AM_INIT_AUTOMAKE([subdir-objects foreign])

# collect arguments
# debug
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[enable debugging @<:@default is yes@:>@]))
# fanotify
AC_ARG_ENABLE([fanotify],
	AS_HELP_STRING([--enable-fanotify],[enable on-access scan using fanotify (see man 7 fanotify) @<:@default is no@:>@.
	The Linux kernel must be compiled with fanotify support]))

# configuration header
AM_CONFIG_HEADER(armadito-config-autoconf.h)

# for now, add builtin modules by hand
# to be move somewhere else
dnl AC_DEFINE([HAVE_ALERT_MODULE],[1],[Define to compile builtin module alert])
dnl AC_DEFINE([HAVE_ONDEMAND_MODULE],[1],[Define to compile builtin module on-demand])
dnl AC_DEFINE([HAVE_QUARANTINE_MODULE],[1],[Define to compile builtin module quarantine])

# This is needed when the first PKG_CHECK_MODULES call is inside a conditional (not the case here, but who knows)
PKG_PROG_PKG_CONFIG

# check for needed programs
AC_PROG_CC([gcc])
AC_PROG_LIBTOOL

# check for functions
AC_CHECK_FUNCS(strdup)
AC_CHECK_FUNCS(open)
AC_CHECK_FUNCS(read)
AC_CHECK_FUNCS(write)
AC_CHECK_FUNCS(close)
AC_CHECK_FUNCS(lseek)
AC_CHECK_FUNCS(getpid)
AC_CHECK_FUNCS(strerror)
AC_CHECK_FUNCS(clock_gettime)

# check for headers
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([syslog.h])

# check for debug
AC_MSG_CHECKING(for debug)
AC_MSG_RESULT($enable_debug)
AM_CONDITIONAL([DEBUG], [test "$enable_debug" = "yes"])
CFLAGS=
if test "$enable_debug" = "yes"; then
   AC_DEFINE(DEBUG,1,[Define DEBUG to enable debug messages])
   CFLAGS="${CFLAGS} -g"
fi

# check for posix thread
AX_PTHREAD

# check for glib stuff
PKG_CHECK_MODULES(GLIB2, glib-2.0, [HAVE_GLIB2=yes], [HAVE_GLIB2=no])
if test "$HAVE_GLIB2" = "no"; then
   AC_MSG_ERROR("cannot find glib2 library")
fi
PKG_CHECK_MODULES(GMODULE2, gmodule-2.0, [HAVE_GMODULE2=yes], [HAVE_GMODULE2=no])
if test "$HAVE_GMODULE2" = "no"; then
   AC_MSG_ERROR("cannot find gmodule2 library")
fi
PKG_CHECK_MODULES(GTHREAD2, gthread-2.0, [HAVE_GTHREAD2=yes], [HAVE_GTHREAD2=no])
if test "$HAVE_GTHREAD2" = "no"; then
   AC_MSG_ERROR("cannot find gthread2 library")
fi
PKG_CHECK_MODULES(GIO2, gio-2.0 gio-unix-2.0, [HAVE_GIO2=yes], [HAVE_GIO2=no])
if test "$HAVE_GIO2" = "no"; then
   AC_MSG_ERROR("cannot find glib gio library")
fi
# check for g_thread_new/g_thread_create
SAVE_CFLAGS=${CFLAGS}
CFLAGS="${GMODULE2_CFLAGS} ${CFLAGS}"
SAVE_LDFLAGS=${LDFLAGS}
LDFLAGS="${GMODULE2_LIBS} ${LDFLAGS}"
AC_CHECK_LIB([glib-2.0], [g_thread_new],
		      [AC_DEFINE([HAVE_GTHREAD_NEW], [], [Define this if glib has the 'g_thread_new' function])])
AC_CHECK_LIB([glib-2.0], [g_thread_create],
		      [AC_DEFINE([HAVE_GTHREAD_CREATE], [], [Define this if glib has the 'g_thread_create' function])])
AC_CHECK_LIB([glib-2.0], [g_thread_init],
		      [AC_DEFINE([HAVE_GTHREAD_INIT], [], [Define this if glib has the 'g_thread_init' function])])
CFLAGS=${SAVE_CFLAGS}
unset SAVE_CFLAGS
LDFLAGS=${SAVE_LDFLAGS}
unset SAVE_LDFLAGS
# for now, disable deprecations, will need more tests in configure.ac
AC_DEFINE([GLIB_DISABLE_DEPRECATION_WARNINGS], [1], [Disable glib deprecation warnings])

# check for libxml2
PKG_CHECK_MODULES(LIBXML2, libxml-2.0, [HAVE_LIBXML2=yes], [HAVE_LIBXML2=no])
if test "$HAVE_LIBXML2" = "no"; then
   AC_MSG_ERROR("cannot find libxml2 library")
fi

# check for libmicrohttpd
PKG_CHECK_MODULES(LIBMICROHTTPD, libmicrohttpd, [HAVE_LIBMICROHTTPD=yes], [HAVE_LIBMICROHTTPD=no])
if test "$HAVE_LIBMICROHTTPD" = "no"; then
   AC_MSG_ERROR("cannot find libmicrohttpd library")
fi

# check for libcurl
PKG_CHECK_MODULES(LIBCURL, libcurl, [HAVE_LIBCURL=yes], [HAVE_LIBCURL=no])
if test "$HAVE_LIBCURL" = "no"; then
   AC_MSG_ERROR("cannot find libcurl library")
fi

# check for libmagic, that has no pkg-config
SAVE_CFLAGS=${CFLAGS}
CFLAGS=-fPIC
AC_CHECK_LIB(magic, magic_open, [HAVE_LIBMAGIC=yes], [HAVE_LIBMAGIC=no])
if test "$HAVE_LIBMAGIC" = "no"; then
   AC_MSG_ERROR("cannot find -lmagic")
fi
CFLAGS=${SAVE_CFLAGS}
unset SAVE_CFLAGS
AC_CHECK_HEADER(magic.h, [HAVE_MAGIC_H=yes], [HAVE_MAGIC_H=no])
if test "$HAVE_MAGIC_H" = "no"; then
   AC_MSG_ERROR("cannot find magic.h")
fi

# check for fanotify
AC_MSG_CHECKING(for on-acces scan using fanotify)
if test "$enable_fanotify" = "no"; then
   AC_MSG_RESULT(disabled)
else
	SAVE_CFLAGS=${CFLAGS}
	CFLAGS=-fPIC
	AC_MSG_RESULT(enabled)
	AC_CHECK_FUNC(fanotify_init, [HAVE_FANOTIFY_INIT=yes], [HAVE_FANOTIFY_INIT=no])
	if test "$HAVE_FANOTIFY_INIT" = "no" ; then
	   AC_MSG_ERROR("fanotify is not available on this Linux distribution but on-access scan using fanotify is enabled")
	fi
	CFLAGS=${SAVE_CFLAGS}
	unset SAVE_CFLAGS
fi
AM_CONDITIONAL([COND_FANOTIFY], [test "$enable_fanotify" = "yes" -a "$HAVE_FANOTIFY_INIT" = "yes"])

# check for jansson library (JSON)
PKG_CHECK_MODULES(LIBJANSSON, jansson, [HAVE_LIBJANSSON=yes], [HAVE_LIBJANSSON=no])
if test "$HAVE_LIBJANSSON" = "no"; then
   AC_MSG_ERROR("cannot find jansson library")
fi

# documentation
# check for asciidoc and a2x
AC_PATH_PROG(ASCIIDOC, asciidoc)
AC_PATH_PROG(A2X, a2x)
AM_CONDITIONAL([COND_MAN], [test ! -z "$ASCIIDOC" -a ! -z "$A2X" ])

# Output files
AC_CONFIG_FILES([
Makefile
arch/Makefile
arch/linux/Makefile
arch/linux/daemon/Makefile
arch/linux/modules/Makefile
arch/linux/modules/on-access-linux/Makefile
arch/linux/modules/on-access-linux/conf/Makefile
arch/linux/modules/on-access-linux/src/Makefile
arch/linux/modules/on-access-linux/tests/Makefile
arch/linux/net/Makefile
arch/linux/service/Makefile
conf/Makefile
httpd/Makefile
libcore/Makefile
libjrpc/Makefile
libjrpc/tests/Makefile
libmodule/Makefile
man/Makefile
man/fr/Makefile
tests/Makefile
tests/web/Makefile
tools/Makefile
])
AC_OUTPUT
