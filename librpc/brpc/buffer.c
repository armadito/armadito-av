void buffer_init(struct buffer *b, size_t initial_size)
{
	if (initial_size == 0)
		initial_size = DEFAULT_INITIAL_SIZE;

	b->base = malloc(initial_size);
	b->filled = b->base;
	b->alloced_end = b->base + initial_size;
}

void buffer_grow(struct buffer *b, size_t needed)
{
	size_t old_size, new_size;

	if (b->filled + needed < b->alloced_end)
		return;

	old_size = b->alloced_end - b->base;
	new_size = old_size;
	while (new_size < old_size + needed)
		new_size *= 2;

	b->base = realloc(b->base, new_size);
	b->filled = b->_base + old_size;
	b->alloced_end = b->base + new_size;
}

