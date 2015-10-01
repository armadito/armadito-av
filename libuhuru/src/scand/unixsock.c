#include "libuhuru-config.h"

#include "unixsock.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <assert.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define UNIX_PATH_MAX	108  /* sic... from taken from /usr/include/linux/un.h */

int server_socket_create(const char *socket_path)
{
  int fd, r;
  struct sockaddr_un listening_addr;
  socklen_t addrlen;
  size_t pathlen;

  pathlen = strlen(socket_path);
  assert(pathlen < UNIX_PATH_MAX);

  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket() failed");
    return -1;
  }

  memset(&listening_addr, 0, sizeof(listening_addr));

  listening_addr.sun_family = AF_UNIX;

  strncpy(listening_addr.sun_path, socket_path, strlen(socket_path));

  addrlen = offsetof(struct sockaddr_un, sun_path) + pathlen + 1;

  if (socket_path[0] == '@')
    listening_addr.sun_path[0] = '\0';

  r = bind(fd, (struct sockaddr *)&listening_addr, addrlen);
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

int server_socket_accept(int server_sock)
{
  struct sockaddr_un client_addr;
  int client_addr_len = sizeof(client_addr);
  int client_sock;

  client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
  
  if (client_sock < 0) {
    perror("accept() failed");
    return -1;
  }

  return client_sock;
}

