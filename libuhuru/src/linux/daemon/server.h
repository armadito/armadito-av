#ifndef _DAEMON_SERVER_H_
#define _DAEMON_SERVER_H_

struct server;

struct server *server_new(struct uhuru *uhuru, int server_sock);

#endif
