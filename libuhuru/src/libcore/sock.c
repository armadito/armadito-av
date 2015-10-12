#include "libuhuru-config.h"

#include "sock.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>

int tcp_socket_connect(const char *hostname, short port_number, int max_retry)
{
  return -1;
}

int unix_socket_connect(const char *path, int max_retry)
{
  return -1;
}

static int do_bind_and_listen(int server_socket)
{
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

int tcp_socket_listen(short port_number, const char *dotted_ip)
{
  return -1;
}


int unix_socket_listen(const char *path)
{
  return -1;
}

static int do_accept(int server_sock, struct sockaddr *addr, socklen_t *p_addrlen)
{
  int client_sock;

  client_sock = accept(server_sock, addr, p_addrlen);
  
  if (client_sock < 0) {
    perror("accept() failed");
    return -1;
  }

  return client_sock;
}

int tcp_socket_accept(int server_sock)
{
  struct sockaddr_in addr;
  int addr_len = sizeof(addr);

  return do_accept(server_sock, &addr, &addr_len);
}

int unix_socket_accept(int server_sock)
{
  struct sockaddr_un addr;
  int addr_len = sizeof(addr);

  return do_accept(server_sock, &addr, &addr_len);
}
