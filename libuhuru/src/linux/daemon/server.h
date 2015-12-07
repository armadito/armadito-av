#ifndef _DAEMON_SERVER_H_
#define _DAEMON_SERVER_H_

struct server;

struct server *server_new(struct uhuru *uhuru, int server_sock);

int server_get_poll_fd(struct server *s);

void server_cb(void *user_data);

#endif
