
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

ssize_t unix_fd_write_cb(const char *buffer, size_t size, void *data);

ssize_t unix_fd_read_cb(char *buffer, size_t size, void *data);
