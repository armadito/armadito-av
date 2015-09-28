#ifndef _UNIXSOCK_H_
#define _UNIXSOCK_H_

#ifndef WIN32
#include <unistd.h>

int client_socket_create(const char *socket_path, int max_retry);

int server_socket_create(const char *socket_path);

int server_socket_accept(int server_sock);

ssize_t read_c(int fd, char *buffer, size_t len);

ssize_t read_n(int fd, char *buffer, size_t len);

ssize_t write_c(int fd, char *buffer, size_t len);

ssize_t write_n(int fd, char *buffer, size_t len);
#endif

#endif
