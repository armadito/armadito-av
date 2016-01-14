#include <libuhuru-config.h>

#include <libuhuru/core.h>

#include "jsonclient.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

struct buffer {
  char *p;
  int first_free;
  int alloc;
};

static void buffer_init(struct buffer *b, int initial_size)
{
  b->first_free = 0;
  b->alloc = initial_size;
  b->p = malloc(b->alloc);
}

static void buffer_destroy(struct buffer *b)
{
  free(b->p);
}

static void buffer_grow(struct buffer *b, int needed)
{
  if (b->first_free + needed >= b->alloc) {
    while (b->first_free + needed >= b->alloc)
      b->alloc *= 2;
    b->p = realloc(b->p, b->alloc);
  }
}

static void buffer_increment(struct buffer *b, int n)
{
  b->first_free += n;
}

static int buffer_length(struct buffer *b)
{
  return b->first_free;
}

static char *buffer_data(struct buffer *b)
{
  return b->p;
}

static char *buffer_end(struct buffer *b)
{
  return b->p + b->first_free;
}

#define INPUT_READ_SIZE 1024

struct json_client {
  struct uhuru *uhuru;
  int sock;
  struct buffer input_buffer;
};

struct json_client *json_client_new(int sock, struct uhuru *uhuru)
{
  struct json_client *cl = malloc(sizeof(struct json_client));
  
  cl->sock = sock;
  cl->uhuru = uhuru;

  buffer_init(&cl->input_buffer, 3 * INPUT_READ_SIZE);

  return cl;
}

void json_client_free(struct json_client *cl)
{
  buffer_destroy(&cl->input_buffer);

  free(cl);
}

int json_client_process(struct json_client *cl)
{
  int n_read, i;

  do {
    /* + 1 for ending null byte */
    buffer_grow(&cl->input_buffer, INPUT_READ_SIZE + 1);

    n_read = read(cl->sock, buffer_end(&cl->input_buffer), INPUT_READ_SIZE);
  
    if (n_read > 0)
      buffer_increment(&cl->input_buffer, n_read);
  } while (n_read > 0);

  if (n_read < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "error in JSON receive: %s", strerror(errno));
    return -1;
  }

  *buffer_end(&cl->input_buffer) = '\0';

  fprintf(stderr, "json_client: received %s\n", buffer_data(&cl->input_buffer));

  return 0;
}
