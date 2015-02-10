#include "protocol.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

enum protocol_handler_state {
  EXPECTING_COMMAND,
  IN_COMMAND,
  EXPECTING_HEADER,
  IN_HEADER,
  EXPECTING_VALUE,
  IN_VALUE,
};

struct protocol_handler {
  enum protocol_handler_state state;
  GString *current_command;
  GString *current_header_key, *current_header_value;
  GHashTable *headers;
};

static void str_free(gpointer p)
{
  g_free(p);
}

struct protocol_handler *protocol_handler_new(void)
{
  struct protocol_handler *h;

  h = (struct protocol_handler *)malloc(sizeof(struct protocol_handler));

  h->state = EXPECTING_COMMAND;
  h->current_command = g_string_new("");
  h->current_header_key = g_string_new("");
  h->current_header_value = g_string_new("");
  h->headers = g_hash_table_new_full(g_str_hash, g_str_equal, str_free, str_free);

  return h;
}

static void protocol_error(struct protocol_handler *h, char c)
{
  fprintf(stderr, "Error in protocol: state %d char %c\n", h->state, c);
}

static void protocol_handler_end_of_header(struct protocol_handler *h)
{
  g_hash_table_insert(h->headers, g_strdup(h->current_header_key->str), g_strdup(h->current_header_value->str));
  g_string_truncate(h->current_header_key, 0);
  g_string_truncate(h->current_header_value, 0);
}

static void GH_print_func(gpointer key, gpointer value, gpointer user_data)
{
  fprintf(stderr, "Header: key=%s value=\"%s\"\n", (char *)key, (char *)value);
}

static void protocol_handler_end_of_command(struct protocol_handler *h)
{
  fprintf(stderr, "Command: %s\n", h->current_command->str);
  g_hash_table_foreach (h->headers, GH_print_func, NULL);
  fprintf(stderr, "\n");

  g_string_truncate(h->current_command, 0);
  g_hash_table_remove_all(h->headers);
  g_string_truncate(h->current_header_key, 0);
  g_string_truncate(h->current_header_value, 0);
}

#define STAY(A) (A)
#define MOVE(A,S) do { A; state = S; } while (0)

int protocol_handler_input_char(struct protocol_handler *h, char c)
{
  switch(h->state) {
  case EXPECTING_COMMAND:
    if (isupper(c) || c == '_')
      g_string_append_c(h->current_command, c);
    else if (c == '\n')
      h->state = EXPECTING_HEADER;
    else
      protocol_error(h, c);
    break;
  case EXPECTING_HEADER:
    if (c == '\n') {
      protocol_handler_end_of_command(h);
      h->state = EXPECTING_COMMAND;
    } else if (isalnum(c) || c == '-' || c == '_')
      g_string_append_c(h->current_header_key, c);
    else if (c == ':') {
      h->state = EXPECTING_VALUE;
    } else
      protocol_error(h, c);
    break;
  case EXPECTING_VALUE:
    if (!isblank(c)) {
      g_string_append_c(h->current_header_value, c);
      h->state = IN_VALUE;
    }
    break;
  case IN_VALUE:
    if (c == '\n') {
      protocol_handler_end_of_header(h);
      h->state = EXPECTING_HEADER;
    } else
      g_string_append_c(h->current_header_value, c);
    break;
  }

  return 0;
}

char *protocol_handler_cmd(struct protocol_handler *handler)
{
  return handler->current_command->str;
}

char *protocol_handler_header(struct protocol_handler *handler, const char *key)
{
  return (char *)g_hash_table_lookup(handler->headers, key);
}


