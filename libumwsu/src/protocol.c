#include "protocol.h"

#include <stdlib.h>

#define PROTOCOL_BUFFER_INITIAL_SIZE 32

struct buffer {
  char *head;
  char *tail;
  int alloc;
};

void buffer_init(struct protocol_buffer *b)
{
  b->alloc = PROTOCOL_BUFFER_INITIAL_SIZE;
  b->head = (char *)malloc(b->alloc);
  b->tail = b->head;
}

void protocol_buffer_destroy(struct protocol_buffer *b)
{
  free(b->buff);
}

void protocol_buffer_grow(struct protocol_buffer *b, int needed)
{
  if (b->head + needed >= b->alloc) {
    while(b->head + needed >= b->alloc)
      b->alloc *= 2;
    b->buff = (char *)realloc(b->buff, b->alloc);
  }
}

void protocol_buffer_append_v(struct protocol_buffer *b, char *p)
{
  while(*p) {
    switch(*p) {
    case '\n':
      protocol_buffer_grow(b, 2);
      b->buff[b->head++] = '\\';
      b->buff[b->head++] = 'n';
      break;
    default:
      protocol_buffer_grow(b, 1);
      b->buff[b->head++] = *p;
      break;
    }

    p++;
  }
}

void protocol_buffer_append_s(struct protocol_buffer *b, char *p, int size)
{
  int i;

  protocol_buffer_grow(b, size);
  
  for(i = 0; i < size; i++)
    b->buff[b->head++] = *p++;
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
      end_of_command();   /* includes reset_headers(); */
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
