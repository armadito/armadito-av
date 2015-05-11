#ifndef _DAEMON_SERVER_H_
#define _DAEMON_SERVER_H_

struct server;

struct server *server_new(void);

void server_loop(struct server *server);

#endif
