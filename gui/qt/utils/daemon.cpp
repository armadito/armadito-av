#include "daemon.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define UNIX_PATH_MAX	108  /* sic... taken from /usr/include/linux/un.h */

int DaemonConnection::connect(int maxRetry)
{
  int _ioFd, r, retry_count = 0;
  struct sockaddr_un connect_addr;
  socklen_t addrlen;
  size_t path_len;

  path_len = strlen(_socketPath);
  assert(path_len < UNIX_PATH_MAX);

  _ioFd = socket( AF_UNIX, SOCK_STREAM, 0);
  if (_ioFd < 0) {
    perror("socket() failed");
    return -1;
  }

  if (maxRetry <= 0)
    maxRetry = 1;

  memset(&connect_addr, 0, sizeof(connect_addr));
  connect_addr.sun_family = AF_UNIX;
  strncpy(connect_addr.sun_path, _socketPath, path_len);

  if (_socketPath[0] == '@')
    connect_addr.sun_path[0] = '\0';

  addrlen = offsetof(struct sockaddr_un, sun_path) + path_len + 1;

  do {
    r = ::connect(_ioFd, (struct sockaddr *)&connect_addr, addrlen);
    retry_count++;
  } while (r < 0 && retry_count <= maxRetry);

  if (r < 0) {
    perror("connect() failed");
    return -1;
  }

  return _ioFd;
}
