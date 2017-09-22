AUTOMAKE_OPTIONS=subdir-objects

check_PROGRAMS=inotify-test mount-test stamp-test queue-test threadpool-test

AM_CFLAGS=-I$(top_srcdir)/libcore/arch/linux/builtin-modules/on-access @GLIB2_CFLAGS@ @GIO2_CFLAGS@ $(PTHREAD_CFLAGS) 
LDADD= $(PTHREAD_CFLAGS) $(PTHREAD_LIBS) @GLIB2_LIBS@ @GIO2_LIBS@

inotify_test_SOURCES=inotify-test.c
mount_test_SOURCES=mount-test.c ../mount.c ../mount.h
queue_test_SOURCES=queue-test.c ../queue.c ../queue.h
stamp_test_SOURCES=stamp-test.c
threadpool_test_SOURCES=threadpool-test.c ../threadpool.c ../threadpool.h
