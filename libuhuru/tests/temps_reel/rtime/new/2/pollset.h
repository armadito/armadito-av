#ifndef _POLLSET_H_
#define _POLLSET_H_

struct poll_set;

struct poll_set *poll_set_new(void);

typedef void (*poll_cb_t)(void *user_data);

int poll_set_add_fd(struct poll_set *s, int fd, poll_cb_t cb, void *user_data);

int poll_set_loop(struct poll_set *s);

#endif
