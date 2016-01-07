AUTOMAKE_OPTIONS=subdir-objects

check_PROGRAMS=inotify-test mount-test

AM_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src/linux/modules/on-access-linux/src @GLIB2_CFLAGS@ @GIO2_CFLAGS@
LDADD= @GLIB2_LIBS@ @GIO2_LIBS@ 

mount_test_SOURCES=mount-test.c ../src/mount.c ../src/mount.h
inotify_test_SOURCES=inotify-test.c 
