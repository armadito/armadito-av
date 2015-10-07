#ifndef _TCPSOCK_H_
#define _TCPSOCK_H_

int client_tcp_socket_create(char *hostname, short port_number, int max_retry);

int server_tcp_socket_create(short port_number, const char *dotted);

#endif

