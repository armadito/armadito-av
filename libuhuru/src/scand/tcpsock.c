#include "tcpsock.h"

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

static in_addr_t get_host_ip(char *hostname)
{
  struct hostent *resolved;
  struct in_addr *ipaddr;

  resolved = gethostbyname(hostname);
  if (resolved == NULL) {
    perror( "host not known");
    return 0;
  }

  ipaddr = (struct in_addr *)resolved->h_addr_list[0];

  return ipaddr->s_addr;
}

int tcp_client_connect(char *hostname, short port_number, int max_retry)
{
  in_addr_t host_ipaddr;
  int fd, r;
  int retry_count = 0;

  fd = socket( AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket() failed");
    return -1;
  }

  host_ipaddr = get_host_ip( hostname);
  if (host_ipaddr == 0) {
    perror("host unknown");
    return -1;
  }

  if (max_retry <= 0)
    max_retry = 1;

  do {
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons( port_number);
    server_addr.sin_addr.s_addr = host_ipaddr;

    r = connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    retry_count++;
  } while (r < 0 && retry_count <= max_retry);

  if (r < 0) {
    perror("connect() failed");
    return -1;
  }

  return fd;
}

int tcp_server_listen(short port_number, const char *dotted)
{
  int fd, optval;
  struct sockaddr_in listening_addr;
  in_addr_t bind_addr = htonl(INADDR_ANY);
  int r;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket() failed");
    return -1;
  }

  optval = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0) {
    perror("setsockopt()");
    return -1;
  }

  if (dotted != NULL)
    bind_addr = inet_addr(dotted);

  listening_addr.sin_family = AF_INET;
  listening_addr.sin_port = htons(port_number);
  listening_addr.sin_addr.s_addr = bind_addr;

  r = bind(fd, (struct sockaddr *)&listening_addr, sizeof(listening_addr));
  if (r < 0) {
    perror("bind() failed");
    return -1;
  }
  
  r = listen(fd, 5);
  if (r < 0) {
    perror("listen() failed");
    return -1;
  }

  return fd;
}
