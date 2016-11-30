
#include "buffer.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define DEFAULT_INITIAL_SIZE 128

void buffer_init(struct buffer *b, size_t initial_size)
{
	b->filled_size = 0;
	if (initial_size == 0)
		b->alloced_size = DEFAULT_INITIAL_SIZE;
	else
		b->alloced_size = initial_size;
	b->p = malloc(b->alloced_size);
}

void buffer_destroy(struct buffer *b)
{
	free(b->p);
}

void buffer_grow(struct buffer *b, size_t needed)
{
	if (b->filled_size + needed >= b->alloced_size) {
		while (b->filled_size + needed >= b->alloced_size)
			b->alloced_size *= 2;
		b->p = realloc(b->p, b->alloced_size);
	}
}

void buffer_increment(struct buffer *b, size_t size)
{
	b->filled_size += size;
}

char *buffer_end(struct buffer *b)
{
	return b->p + b->filled_size;
}

void buffer_append(struct buffer *b, const char *data, size_t size)
{
	buffer_grow(b, size);

	memcpy(buffer_end(b), data, size);

	buffer_increment(b, size);
}

size_t buffer_size(struct buffer *b)
{
	return b->filled_size;
}

char *buffer_data(struct buffer *b)
{
	return b->p;
}

void buffer_clear(struct buffer *b)
{
	b->filled_size = 0;
}
