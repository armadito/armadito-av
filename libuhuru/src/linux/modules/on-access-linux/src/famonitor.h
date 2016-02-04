#ifndef _FAMONITOR_H_
#define _FAMONITOR_H_

#include <libuhuru/core.h>

struct fanotify_monitor;

struct fanotify_monitor *fanotify_monitor_new(struct uhuru *u);

int fanotify_monitor_start(struct fanotify_monitor *f);

int fanotify_monitor_mark_directory(struct fanotify_monitor *f, const char *path, int enable_permission);

int fanotify_monitor_unmark_directory(struct fanotify_monitor *f, const char *path, int enable_permission);

int fanotify_monitor_mark_mount(struct fanotify_monitor *f, const char *path, int enable_permission);

int fanotify_monitor_unmark_mount(struct fanotify_monitor *f, const char *path, int enable_permission);

#endif
