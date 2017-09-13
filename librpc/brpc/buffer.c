#include "buffer.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_INITIAL_SIZE 128

void buffer_init(struct buffer *b, size_t initial_size)
{
	if (initial_size == 0)
		initial_size = DEFAULT_INITIAL_SIZE;

	b->base = malloc(initial_size);
	b->end = b->base;
	b->alloced_end = b->base + initial_size;
}

void buffer_destroy(struct buffer *b, int free_data)
{
	b->end = NULL;
	b->alloced_end = NULL;
	if (free_data)
		free(b->base);
	b->base = NULL;
}

struct buffer *buffer_new(size_t initial_size)
{
	struct buffer *b = malloc(sizeof(struct buffer));

	buffer_init(b, initial_size);

	return b;
}

void buffer_free(struct buffer *b, int free_data)
{
	buffer_destroy(b, free_data);
	free(b);
}

void buffer_make_room(struct buffer *b, size_t needed)
{
	size_t available, old_size, new_size;

	available = b->alloced_end - b->end;
	if (needed <= available)
		return;

	old_size = b->alloced_end - b->base;
	new_size = old_size;
	while (new_size < old_size + needed)
		new_size *= 2;

	b->base = realloc(b->base, new_size);
	b->end = b->base + old_size;
	b->alloced_end = b->base + new_size;
}

void buffer_append(struct buffer *b, const void *data, size_t size)
{
	buffer_make_room(b, size);

	memcpy(buffer_end(b), data, size);

	buffer_increment(b, size);
}

void buffer_str_append(struct buffer *b, const char *s)
{
	const char *p = s;

	do {
		buffer_make_room(b, 1);
		*((char *)buffer_end(b)) = *p;
		buffer_increment(b, 1);
	} while (*p++);
}

void buffer_fill(struct buffer *b, int c, size_t size)
{
	buffer_make_room(b, size);

	memset(buffer_end(b), c, size);

	buffer_increment(b, size);
}

