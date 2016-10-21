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

#include <libarmadito.h>
#include "armadito-config.h"

#include "os/mimetype.h"

#include <glib.h>
#include <magic.h>
#include <string.h>
#include <unistd.h>

/* Unfortunately, libmagic is not thread-safe. */
/* We create a new magic_t for each thread, and keep it  */
/* in thread's private data, so that it is created only once. */

static int init_done = 0;
static GPrivate *private_key;

static void magic_destroy_notify(gpointer data)
{
	magic_close((magic_t)data);
}

static magic_t get_private_magic(void)
{
	magic_t m;

	m = (magic_t)g_private_get(private_key);

	if (m == NULL) {
		m = magic_open(MAGIC_MIME_TYPE);
		magic_load(m, NULL);

		g_private_set(private_key, (gpointer)m);
	}

	return m;
}

void os_mime_type_init(void)
{
	if (!init_done) {
		private_key = g_private_new(magic_destroy_notify);

		// this is to pre-load the magic file for the current thread
		// to avoid a dead-lock for the first scanned file
		get_private_magic();

		init_done = 1;
	}
}

const char *os_mime_type_guess(const char *path)
{
	magic_t m;
	const char *mime_type;

	m = get_private_magic();

	mime_type = magic_file(m, path);

	return strdup(mime_type);
}

#define BUFFER_SIZE 1024

const char *os_mime_type_guess_fd(int fd)
{
	magic_t m;
	const char *mime_type;
	char *buffer[BUFFER_SIZE];
	int n_read;

	m = get_private_magic();

	if ((n_read = read(fd, buffer, BUFFER_SIZE)) < 0) {
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "cannot read %d bytes from file descriptor %d", BUFFER_SIZE, fd);
		return NULL;
	}

	mime_type = magic_buffer(m, buffer, n_read);

	return strdup(mime_type);
}
