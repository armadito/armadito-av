#ifndef _FAWATCH_H_
#define _FAWATCH_H_

struct fa_watch;

struct fa_watch *fa_watch_new(void);

int fa_watch_add(struct fa_watch *w, const char *path);

int fa_watch_remove(struct fa_watch *w, const char *path);

void fa_watch_free(struct fa_watch *w);

#endif
