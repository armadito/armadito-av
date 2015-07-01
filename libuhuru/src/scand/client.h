#ifndef _DAEMON_CLIENT_H_
#define _DAEMON_CLIENT_H_

#include <libumwsu/scan.h>

struct client;

struct client *client_new(int client_sock, struct umwsu *umwsu);

int client_process(struct client *cl);

#endif
