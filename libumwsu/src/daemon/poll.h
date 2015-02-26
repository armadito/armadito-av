#ifndef _DAEMON_POLL_H_
#define _DAEMON_POLL_H_

struct poll_set;

typedef int (*poll_cb_t)(int sock, void *data);

struct poll_set *poll_set_new(int listen_sock, poll_cb_t cb);

int poll_set_loop(struct poll_set *s, void *data);

void poll_set_close(struct poll_set *s);

#endif
