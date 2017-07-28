#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

struct buffer {
	char *base;
	char *filled;
	char *alloced_end;
};

void buffer_init(struct buffer *b, size_t initial_size);

void buffer_destroy(struct buffer *b, int free_data);

struct buffer *buffer_new(size_t initial_size);

void buffer_free(struct buffer *b, int free_data);

void buffer_make_room(struct buffer *b, size_t needed);

static inline int buffer_overflow(struct buffer *b, size_t needed)
{
	return b->filled + needed >= b->alloced_end;
}

static inline void buffer_increment(struct buffer *b, size_t size)
{
	b->filled += size;
}

static inline char *buffer_end(struct buffer *b)
{
	return b->filled;
}

static inline size_t buffer_size(struct buffer *b)
{
	return b->filled - b->base;
}

static inline char *buffer_data(struct buffer *b)
{
	return b->base;
}

static inline void buffer_clear(struct buffer *b)
{
	b->filled = b->base;
}

void buffer_append(struct buffer *b, const char *data, size_t size);

void buffer_fill(struct buffer *b, int c, size_t size);

#endif
