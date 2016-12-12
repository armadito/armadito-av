#include <assert.h>
#include <unistd.h>

ssize_t unix_fd_write_cb(const char *buffer, size_t size, void *data)
{
	int fd = *(int *)data;
	size_t to_write = size;

	assert(size > 0);

	while (to_write > 0) {
		int w = write(fd, buffer, to_write);

		if (w < 0)
			return 1;

		if (w == 0)
			continue;

		buffer += w;
		to_write -= w;
	}

	return 0;
}

ssize_t unix_fd_read_cb(char *buffer, size_t size, void *data)
{
	int fd = *(int *)data;

	return read(fd, buffer, size);
}

