#include "protocol.h"

#include <stdlib.h>

struct vector {
  int element_size;
  int alloc;
  char *head;
  char *tail;
};

#define DEFAULT_RESERVED_SIZE 256

void vector_init(struct vector *v, int element_size, int reserved_size)
{
  v->element_size = element_size;
  if (reserved_size == 0)
    reserved_size = DEFAULT_RESERVED_SIZE;
  v->alloc = reserved_size * v->element_size;
  v->head = (char *)malloc(v->alloc);
  v->tail = v->head;
}

void vector_grow(struct vector *v, int needed)
{
  int current_size = v->tail - v->head;
  int needed_size = needed * v->element_size;

  if (current_size + needed_size >= v->alloc) {
    while (current_size + needed_size >= v->alloc)
      v->alloc *= 2;
    v->head = (char *)realloc(v->head, v->alloc);
    v->tail = v->head + current_size;
  }
}

void *vector_append(struct vector *v, int n)
{
  void *tail;

  vector_grow(v, n);

  tail = v->tail;
  v->tail += n * v->element_size;

  return tail;
}

void vector_clear(struct vector *v)
{
  v->tail = v->head;
}

int vector_size(struct vector *v)
{
  return (v->tail - v->head) / v->element_size;
}

void *vector_head(struct vector *v)
{
  return v->head;
}

void *vector_tail(struct vector *v)
{
  return v->tail;
}

void *vector_index(struct vector *v, int i)
{
  return v->head + i * v->element_size;
}

struct header {
  struct vector name;
  struct vector value;
};

struct command {
  struct vector name;
  struct vector headers;
};

void command_init(struct command *c)
{
  
}

void command_append_name(struct command *c, char p)
{
}

void header_append_name(struct header *h, char *p)
{
}

void header_append_name(struct header *h, char *p)
{
}

char *header_name(struct header *h)
{
  return NULL;
}

char *header_value(struct header *h)
{
  return NULL;
}

#define STAY(A) (A)
#define MOVE(A,S) do { A; state = S; } while (0)

void protocol_decode(struct protocol_hander *h)
{
  switch(state) {
  case IN_COMMAND:
    if (isalpha(c))
      append_to_command(c);
    else if (c == '\n') {
      append_to_command('\0');
      reset_headers();
      state = IN_HEADER_NAME;
    } else
      protocol_error(c);
    break;
  case IN_HEADER_NAME:
    if (isalpha(c))
      append_to_header_name(c);
    else if (c == ':') {
      append_to_header_name('\0');
      state = IN_HEADER_VALUE_SKIP_SPACE;
    } else if (c == '\n') {
      end_of_command();
      state = IN_COMMAND;
    } else
      protocol_error(c);
    break;
  case IN_HEADER_VALUE_SKIP_SPACE:
    if (isalpha(c)) {
      append_to_header_value(c);
      state = IN_HEADER_VALUE;
    } else if (is_space(c))
      ;
    else
      protocol_error(c);
    break;
  case IN_HEADER_VALUE:
    if (isalpha(c)) {
      append_to_header_value(c);
    } else if (c == '\n') {
      append_to_header_value('\0');
      next_header();
      state = IN_HEADER_NAME;
    } else
      protocol_error(c);
    break;
  }

}
