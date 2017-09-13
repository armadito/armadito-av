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

#include "buffer.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define DEFAULT_INITIAL_SIZE 128

static void buffer_init(struct buffer *b, size_t initial_size)
{
	b->filled_size = 0;
	if (initial_size == 0)
		b->alloced_size = DEFAULT_INITIAL_SIZE;
	else
		b->alloced_size = initial_size;
	b->p = malloc(b->alloced_size);
}

static void buffer_destroy(struct buffer *b)
{
	free(b->p);
}

static void buffer_grow(struct buffer *b, size_t needed)
{
	if (b->filled_size + needed >= b->alloced_size) {
		while (b->filled_size + needed >= b->alloced_size)
			b->alloced_size *= 2;
		b->p = realloc(b->p, b->alloced_size);
	}
}

static void buffer_increment(struct buffer *b, size_t size)
{
	b->filled_size += size;
}

static char *buffer_end(struct buffer *b)
{
	return b->p + b->filled_size;
}

static void buffer_append(struct buffer *b, const char *data, size_t size)
{
	buffer_grow(b, size);

	memcpy(buffer_end(b), data, size);

	buffer_increment(b, size);
}

static size_t buffer_size(struct buffer *b)
{
	return b->filled_size;
}

static char *buffer_data(struct buffer *b)
{
	return b->p;
}

static void buffer_clear(struct buffer *b)
{
	b->filled_size = 0;
}
