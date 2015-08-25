#include "unixsock.h"

#include <assert.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

int client_socket_create(const char *socket_path, int max_retry)
{
  int fd, r, retry_count = 0;
  struct sockaddr_un server_addr;

  fd = socket( AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket() failed");
    return -1;
  }

  if (max_retry <= 0)
    max_retry = 1;

  server_addr.sun_family = AF_UNIX;
  strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

  do {
    r = connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    retry_count++;
  } while (r < 0 && retry_count <= max_retry);

  if (r < 0) {
    perror("connect() failed");
    return -1;
  }

  return fd;
}

int server_socket_create(const char *socket_path)
{
  int fd, r;
  struct sockaddr_un listening_addr;

  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket() failed");
    return -1;
  }

  listening_addr.sun_family = AF_UNIX;
  strncpy(listening_addr.sun_path, socket_path, sizeof(listening_addr.sun_path) - 1);

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

ssize_t read_c(int fd, char *buffer, size_t len)
{
  ssize_t nread = read(fd, buffer, len);

  if (nread < 0) {
    perror("read");
    exit(EXIT_FAILURE);
  }

  return nread;
}

ssize_t read_n(int fd, char *buffer, size_t len)
{
  size_t to_read = len;

  assert(len > 0);

  while (to_read > 0) {
    int r = read_c(fd, buffer, to_read);

    if (r == 0)
      return 0;

    buffer += r;
    to_read -= r;
  }

  return len;
}

ssize_t write_c(int fd, char *buffer, size_t len)
{
  ssize_t nwrite = write(fd, buffer, len);

  if (nwrite < 0) {
    perror("write");
    exit(EXIT_FAILURE);
  }

  return nwrite;
}

ssize_t write_n(int fd, char *buffer, size_t len)
{
  size_t to_write = len;

  assert(len > 0);

  while (to_write > 0) {
    int w = write_c(fd, buffer, to_write);

    if (w == 0)
      return 0;

    buffer += w;
    to_write -= w;
  }

  return len;
}
