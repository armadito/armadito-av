/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "unixsockclient.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define UNIX_PATH_MAX	108  /* sic... taken from /usr/include/linux/un.h */

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

	if (r < 0)
		return r;

	return fd;
}
