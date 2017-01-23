AUTOMAKE_OPTIONS=subdir-objects

check_PROGRAMS=inotify-test stamp-test #queue-test mount-test

AM_CFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/linux/modules/on-access-linux/src @GLIB2_CFLAGS@ @GIO2_CFLAGS@
LDADD= @GLIB2_LIBS@ @GIO2_LIBS@

#mount_test_SOURCES=mount-test.c ../src/mount.c ../src/mount.h
inotify_test_SOURCES=inotify-test.c

stamp_test_SOURCES=stamp-test.c

#queue_test_SOURCES=queue-test.c ../src/queue.c ../src/queue.h
