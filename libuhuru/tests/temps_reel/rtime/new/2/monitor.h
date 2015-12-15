#ifndef _MONITOR_H_
#define _MONITOR_H_

struct access_monitor;

enum access_monitor_flags {
  MONITOR_MOUNT        = 1 << 0,
  MONITOR_RECURSIVE    = 1 << 1,
  MONITOR_TYPE_CHECK   = 1 << 2,
  MONITOR_LOG_EVENT    = 1 << 3,
  MONITOR_ENABLE_PERM  = 1 << 4,
};

struct access_monitor *access_monitor_new(enum access_monitor_flags flags);

int access_monitor_get_poll_fd(struct access_monitor *m);

void access_monitor_cb(void *user_data);

int access_monitor_add(struct access_monitor *m, const char *path, unsigned int flags);

#if 0
int access_monitor_remove(struct access_monitor *m, const char *path);
#endif

void access_monitor_free(struct access_monitor *m);

#if 0
int access_monitor_activate(struct access_monitor *m);
#endif

#endif
