#include "os/ipc.h"

#include <assert.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

#define UNIX_PATH_MAX	108  /* sic... from taken from /usr/include/linux/un.h */

int os_ipc_connect(const char *url, int max_retry)
{
  int fd, r, retry_count = 0;
  struct sockaddr_un server_addr;
  const char *socket_path = url;
  socklen_t addrlen;
  size_t pathlen;

  pathlen = strlen(socket_path);
  assert(pathlen < UNIX_PATH_MAX);

  fd = socket( AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket() failed");
    return -1;
  }

  if (max_retry <= 0)
    max_retry = 1;

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sun_family = AF_UNIX;

  strncpy(server_addr.sun_path, socket_path, strlen(socket_path));

  addrlen = offsetof(struct sockaddr_un, sun_path) + pathlen + 1;

  if (socket_path[0] == '@')
    server_addr.sun_path[0] = '\0';

  do {
    r = connect(fd, (struct sockaddr *)&server_addr, addrlen);
    retry_count++;
  } while (r < 0 && retry_count <= max_retry);

  if (r < 0) {
    perror("connect() failed");
    return -1;
  }

  return fd;
}

