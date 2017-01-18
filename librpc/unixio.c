#include "rpc/io.h"

#include <sys/types.h>
#include <sys/socket.h>

ssize_t unix_fd_write_cb(const char *buffer, size_t size, void *data)
{
	int fd = *(int *)data;

	return send(fd, buffer, size, MSG_EOR);
}

ssize_t unix_fd_read_cb(char *buffer, size_t size, void *data)
{
	int fd = *(int *)data;

	return recv(fd, buffer, size, 0);
}
