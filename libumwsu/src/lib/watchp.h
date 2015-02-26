#ifndef _LIBUMWSU_WATCHP_H_
#define _LIBUMWSU_WATCHP_H_

struct umwsu_watch;

struct umwsu_watch *umwsu_watch_new(void);

void umwsu_watch_add(struct umwsu_watch *w, const char *dir);

int umwsu_watch_wait(struct umwsu_watch *w, struct umwsu_watch_event *event);

#endif

