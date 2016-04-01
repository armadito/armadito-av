#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <glib.h>

struct access_monitor;

struct access_monitor *access_monitor_new(struct armadito *armadito);

int access_monitor_enable(struct access_monitor *m, int enable);

int access_monitor_is_enable(struct access_monitor *m);

int access_monitor_enable_permission(struct access_monitor *m, int enable_permission);

int access_monitor_is_enable_permission(struct access_monitor *m);

int access_monitor_enable_removable_media(struct access_monitor *m, int enable_removable_media);

int access_monitor_is_enable_removable_media(struct access_monitor *m);

GMainContext *access_monitor_get_main_context(struct access_monitor *m);

void access_monitor_add_mount(struct access_monitor *m, const char *mount_point);

void access_monitor_add_directory(struct access_monitor *m, const char *path);

int access_monitor_recursive_mark_directory(struct access_monitor *m, const char *path);

int access_monitor_unmark_directory(struct access_monitor *m, const char *path);

int access_monitor_delayed_start(struct access_monitor *m);

#define ACCESS_MONITOR_START    ((char)0x1)
#define ACCESS_MONITOR_STOP     ((char)0x2)
#define ACCESS_MONITOR_STATUS   ((char)0x3)

int access_monitor_send_command(struct access_monitor *m, char command);

#endif
