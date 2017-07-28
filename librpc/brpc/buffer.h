#ifndef BUFFER_H
#define BUFFER_H

struct buffer {
	char *base;
	char *filled;
	char *alloced_end;
};

void buffer_init(struct buffer *b, size_t initial_size);

void buffer_destroy(struct buffer *b);

void buffer_grow(struct buffer *b, size_t needed);

inline void buffer_increment(struct buffer *b, size_t size)
{
	b->filled += size;
}

inline char *buffer_end(struct buffer *b)
{
	return b->filled;
}

inline size_t buffer_size(struct buffer *b)
{
	return b->filled - b->base;
}

inline char *buffer_data(struct buffer *b)
{
	return b->base;
}

void buffer_clear(struct buffer *b);

#endif
