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

#include "armadito-config.h"

#include <libarmadito/armadito.h>

#include <stdarg.h>

#define DEFAULT_INITIAL_SIZE 128

struct buffer {
	size_t alloced_size;
	size_t filled_size;
	char *buff;
};

static void init(struct buffer *b)
{
	b->alloced_size = DEFAULT_INITIAL_SIZE;
	b->buff = malloc(b->alloced_size);
	b->filled_size = 0;
}

static void grow_by_1(struct buffer *b)
{
	if (b->filled_size + 1 >= b->alloced_size) {
		b->alloced_size *= 2;
		b->buff = realloc(b->buff, b->alloced_size);
	}
}

static void append(struct buffer *b, char c)
{
	grow_by_1(b);
	b->buff[b->filled_size] = c;
	b->filled_size++;
}

char *a6o_strcat_(const char *src, ...)
{
	va_list args;
	const char *current_arg;
	struct buffer b;

	if (src == NULL)
		return NULL;

	init(&b);

	va_start(args, src);
	current_arg = src;
	while (current_arg != NULL) {
		const char *p = current_arg;

		while (*p) {
			append(&b, *p);
			p++;
		}

		current_arg = va_arg(args, char *);
	}

	append(&b, '\0');
	va_end(args);

	return b.buff;
}

