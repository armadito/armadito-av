#include "unixsockserver.h"

#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#define UNIX_PATH_MAX	108  /* sic... taken from /usr/include/linux/un.h */

int unix_server_listen(const char *socket_path)
{
	int fd;
	struct sockaddr_un listening_addr;
	socklen_t addrlen;
	size_t path_len;

	path_len = strlen(socket_path);
	assert(path_len < UNIX_PATH_MAX);

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("socket() failed");
		return -1;
	}

	memset(&listening_addr, 0, sizeof(listening_addr));
	listening_addr.sun_family = AF_UNIX;
	strncpy(listening_addr.sun_path, socket_path, path_len);

	/* is socket_path abstract? (see man 7 unix) */
	if (socket_path[0] == '@')
		listening_addr.sun_path[0] = '\0';

	if (socket_path[0] != '\0')
		unlink(socket_path);

	addrlen = offsetof(struct sockaddr_un, sun_path) + path_len + 1;

	if (bind(fd, (struct sockaddr *)&listening_addr, addrlen) < 0) {
		perror("bind() failed");
		return -1;
	}

	if (listen(fd, 5) < 0) {
		perror("listen() failed");
		return -1;
	}

	if (socket_path[0] != '\0')
		chmod(socket_path, 0777);

	return fd;
}

