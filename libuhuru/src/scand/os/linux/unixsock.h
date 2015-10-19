#ifndef _UNIXSOCK_H_
#define _UNIXSOCK_H_

int server_socket_create(const char *socket_path);

int server_socket_accept(int server_sock);

#endif
