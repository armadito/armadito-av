#ifndef _TCPSOCK_H_
#define _TCPSOCK_H_

int tcp_server_listen(short port_number, const char *dotted);

/* not used in daemon */
int tcp_client_connect(char *hostname, short port_number, int max_retry);

#endif

