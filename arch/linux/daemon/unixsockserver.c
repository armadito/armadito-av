/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

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

	if ((fd = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0) {
		perror("socket() failed");
		return -1;
	}

	listening_addr.sun_family = AF_UNIX;
	memset(&listening_addr.sun_path, 0, sizeof(listening_addr.sun_path));
	strncpy(listening_addr.sun_path, socket_path, path_len);

	/* is socket_path abstract? (see man 7 unix) */
	if (socket_path[0] == '@')
		listening_addr.sun_path[0] = '\0';

	if (socket_path[0] != '@')
		unlink(socket_path);

	/* addrlen = offsetof(struct sockaddr_un, sun_path) + path_len + 1; */
	addrlen = sizeof(struct sockaddr_un);

	if (bind(fd, (struct sockaddr *)&listening_addr, addrlen) < 0) {
		close(fd);
		perror("bind() failed");
		return -1;
	}

	if (listen(fd, 5) < 0) {
		close(fd);
		perror("listen() failed");
		return -1;
	}

	/* TODO : use guid instead of giving full access */
	if (socket_path[0] != '@' && chmod(socket_path, 0777) < 0)
		perror("chmod on socket failed");

	return fd;
}

