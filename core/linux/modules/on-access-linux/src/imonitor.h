#ifndef _IMONITOR_H_
#define _IMONITOR_H_

#include "monitor.h"

struct inotify_monitor;

struct inotify_monitor *inotify_monitor_new(struct access_monitor *m);

int inotify_monitor_start(struct inotify_monitor *im);

int inotify_monitor_mark_directory(struct inotify_monitor *im, const char *path);

int inotify_monitor_unmark_directory(struct inotify_monitor *im, const char *path);

#endif
