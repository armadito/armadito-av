#ifndef _FANOTIFY_H_
#define _FANOTIFY_H_

struct watchd;

struct watchd *watchd_new(void);

void watchd_free(struct watchd *w);

#endif
