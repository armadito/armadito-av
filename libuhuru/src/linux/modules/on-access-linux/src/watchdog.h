#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

struct watchdog;

/* Standard value of timeout (in micro seconds) */
#define TIMEOUT  1000  

struct watchdog *watchdog_new(int fanotify_fd);

void watchdog_add(struct watchdog *w, int fd);

int watchdog_remove(struct watchdog *w, int fd, struct timespec *after);

#endif
