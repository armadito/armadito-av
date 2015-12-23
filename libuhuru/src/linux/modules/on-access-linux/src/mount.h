#ifndef _ONACCESS_MOUNT_H_
#define _ONACCESS_MOUNT_H_

struct mount_monitor;

enum mount_event_type {
  EVENT_MOUNT,
  EVENT_UMOUNT,
};

typedef void (*mount_cb_t)(enum mount_event_type ev_type, const char *path, void *user_data);

struct mount_monitor *mount_monitor_new(mount_cb_t cb, void *user_data);

void mount_monitor_free(struct mount_monitor *m);

#endif
