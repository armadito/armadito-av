#ifndef _DAEMON_CLIENT_H_
#define _DAEMON_CLIENT_H_

struct client;

struct client *client_new(int client_sock);

int client_process(struct client *cl);

#endif
