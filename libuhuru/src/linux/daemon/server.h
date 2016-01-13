#ifndef _DAEMON_SERVER_H_
#define _DAEMON_SERVER_H_

enum ipc_type {
  OLD_IPC,
  JSON_IPC,
};

struct server;

struct server *server_new(struct uhuru *uhuru, int server_sock, enum ipc_type ipc_type);

int server_get_poll_fd(struct server *s);

void server_cb(void *user_data);

#endif
