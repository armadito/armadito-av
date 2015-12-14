#include <magic.h>
#include <string.h>
#include <stdio.h>

/* Unfortunately, libmagic is not thread-safe. */
static magic_t m;

void mime_type_init(void)
{
  m = magic_open(MAGIC_MIME_TYPE);
  magic_load(m, NULL);
}

#define BUFFER_SIZE 1024

const char *mime_type_guess_fd(int fd)
{
  const char *mime_type = "application/zob";
  char *buffer[BUFFER_SIZE];
  int n_read;

  if ((n_read = read(fd, buffer, BUFFER_SIZE)) < 0) {
    fprintf(stderr, "cannot read %d bytes from file descriptor %d", BUFFER_SIZE, fd);
    return NULL;
  }

  mime_type = magic_buffer(m, buffer, n_read);

  return strdup(mime_type);
}
