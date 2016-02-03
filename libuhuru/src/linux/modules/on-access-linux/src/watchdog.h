#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

struct watchdog;

/* Standard value of timeout: */
/* TIMEOUT is in micro seconds */
#define TIMEOUT  1000  

struct watchdog *watchdog_new(int fanotify_fd);

void watchdog_add(struct watchdog *w, int fd, const char *path);

int watchdog_remove(struct watchdog *w, int fd, const char **p_path, struct timespec *after);

#endif
