#ifndef _SOCK_H_
#define _SOCK_H_

int tcp_socket_connect(const char *hostname, short port_number, int max_retry);

int unix_socket_connect(const char *path, int max_retry);

int tcp_socket_listen(short port_number, const char *dotted_ip);

int unix_socket_listen(const char *path);

int tcp_socket_accept(int server_sock);

int unix_socket_accept(int server_sock);

ssize_t read_c(int fd, char *buffer, size_t len);

ssize_t read_n(int fd, char *buffer, size_t len);

ssize_t write_c(int fd, char *buffer, size_t len);

ssize_t write_n(int fd, char *buffer, size_t len);

#endif
