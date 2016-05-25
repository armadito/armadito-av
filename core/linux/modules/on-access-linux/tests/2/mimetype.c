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

#include <magic.h>
#include <string.h>
#include <stdio.h>

/* Unfortunately, libmagic is not thread-safe. */
static magic_t m;

void mime_type_init(void)
{
	m = magic_open(MAGIC_MIME_TYPE);
	magic_load(m, NULL);
}

#define BUFFER_SIZE 1024

const char *mime_type_guess_fd(int fd)
{
	const char *mime_type = "application/zob";
	char *buffer[BUFFER_SIZE];
	int n_read;

	if ((n_read = read(fd, buffer, BUFFER_SIZE)) < 0) {
		fprintf(stderr, "cannot read %d bytes from file descriptor %d", BUFFER_SIZE, fd);
		return NULL;
	}

	mime_type = magic_buffer(m, buffer, n_read);

	return strdup(mime_type);
}
