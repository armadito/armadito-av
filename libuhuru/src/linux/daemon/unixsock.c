#include "unixsock.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>

#define UNIX_PATH_MAX	108  /* sic... taken from /usr/include/linux/un.h */

int unix_server_listen(const char *socket_path)
{
  int fd, r;
  struct sockaddr_un listening_addr;
  socklen_t addrlen;
  size_t path_len;

  path_len = strlen(socket_path);
  assert(path_len < UNIX_PATH_MAX);

  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket() failed");
    return -1;
  }

  memset(&listening_addr, 0, sizeof(listening_addr));
  listening_addr.sun_family = AF_UNIX;
  strncpy(listening_addr.sun_path, socket_path, path_len);

  /* is socket_path abstract? (see man 7 unix) */
  if (socket_path[0] == '@')
    listening_addr.sun_path[0] = '\0';
  else
    unlink(socket_path);

  addrlen = offsetof(struct sockaddr_un, sun_path) + path_len + 1;

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

int unix_client_connect(const char *socket_path, int max_retry)
{
  int fd, r, retry_count = 0;
  struct sockaddr_un connect_addr;
  socklen_t addrlen;
  size_t path_len;

  path_len = strlen(socket_path);
  assert(path_len < UNIX_PATH_MAX);

  fd = socket( AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket() failed");
    return -1;
  }

  if (max_retry <= 0)
    max_retry = 1;

  memset(&connect_addr, 0, sizeof(connect_addr));
  connect_addr.sun_family = AF_UNIX;
  strncpy(connect_addr.sun_path, socket_path, path_len);

  if (socket_path[0] == '@')
    connect_addr.sun_path[0] = '\0';

  addrlen = offsetof(struct sockaddr_un, sun_path) + path_len + 1;

  do {
    r = connect(fd, (struct sockaddr *)&connect_addr, addrlen);
    retry_count++;
  } while (r < 0 && retry_count <= max_retry);

  if (r < 0) {
    perror("connect() failed");
    return -1;
  }

  return fd;
}

