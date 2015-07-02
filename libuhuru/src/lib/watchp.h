#ifndef _LIBUHURU_WATCHP_H_
#define _LIBUHURU_WATCHP_H_

struct uhuru_watch;

struct uhuru_watch *uhuru_watch_new(void);

void uhuru_watch_add(struct uhuru_watch *w, const char *dir);

int uhuru_watch_wait(struct uhuru_watch *w, struct uhuru_watch_event *event);

#endif

