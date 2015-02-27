#ifndef _DAEMON_POLL_H_
#define _DAEMON_POLL_H_

struct poll_set;

struct poll_set *poll_set_new(int listen_sock);

int poll_set_loop(struct poll_set *s);

void poll_set_close(struct poll_set *s);

#endif
