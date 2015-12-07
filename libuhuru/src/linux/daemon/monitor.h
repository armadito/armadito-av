#ifndef _MONITOR_H_
#define _MONITOR_H_

struct access_monitor;

struct access_monitor *access_monitor_new(struct uhuru *uhuru);

int access_monitor_get_poll_fd(struct access_monitor *m);

void access_monitor_cb(void *user_data);

int access_monitor_enable_permission(struct access_monitor *m, int enable_permission);

int access_monitor_add(struct access_monitor *m, const char *path);

int access_monitor_remove(struct access_monitor *m, const char *path);

void access_monitor_free(struct access_monitor *m);

int access_monitor_activate(struct access_monitor *m);

#endif
