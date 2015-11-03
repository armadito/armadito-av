#ifndef _UNIXSOCK_H_
#define _UNIXSOCK_H_

int unix_server_listen(const char *socket_path);

int unix_client_connect(const char *socket_path, int max_retry);

#endif
