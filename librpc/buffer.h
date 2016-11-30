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

#ifndef LIBRPC_BUFFER_H
#define LIBRPC_BUFFER_H

#include <stddef.h>

struct buffer {
	char *p;
	size_t filled_size;
	size_t alloced_size;
};

void buffer_init(struct buffer *b, size_t initial_size);

void buffer_destroy(struct buffer *b);

void buffer_grow(struct buffer *b, size_t needed);

void buffer_increment(struct buffer *b, size_t size);

char *buffer_end(struct buffer *b);

void buffer_append(struct buffer *b, const char *data, size_t size);

size_t buffer_size(struct buffer *b);

char *buffer_data(struct buffer *b);

void buffer_clear(struct buffer *b);

#endif
