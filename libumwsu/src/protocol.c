#include "protocol.h"
#include "libumwsu-config.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

enum protocol_handler_state {
  EXPECTING_COMMAND,
  IN_COMMAND,
  EXPECTING_HEADER,
  IN_HEADER,
  EXPECTING_VALUE,
  IN_VALUE,
};

struct protocol_handler {
  FILE *input;
  FILE *output;
  enum protocol_handler_state state;
  GString *current_command;
  GString *current_header_key, *current_header_value;
  GHashTable *current_headers;
  GHashTable *callbacks;
};

static void str_free(gpointer p)
{
  g_free(p);
}

struct protocol_handler *protocol_handler_new(int input_fd, int output_fd)
{
  struct protocol_handler *h;

  h = (struct protocol_handler *)malloc(sizeof(struct protocol_handler));

  h->input = fdopen(input_fd, "r");
  h->output = fdopen(output_fd, "w");
  if (h->input == NULL || h->output == NULL) {
    perror("fdopen");
    free(h);
    return NULL;
  }
  h->state = EXPECTING_COMMAND;
  h->current_command = g_string_new("");
  h->current_header_key = g_string_new("");
  h->current_header_value = g_string_new("");
  h->current_headers = g_hash_table_new_full(g_str_hash, g_str_equal, str_free, str_free);

  h->callbacks = g_hash_table_new(g_str_hash, g_str_equal);

  return h;
}

void protocol_handler_free(struct protocol_handler *handler)
{
  g_string_free(handler->current_command, TRUE);
  g_string_free(handler->current_header_key, TRUE);
  g_string_free(handler->current_header_value, TRUE);

  g_hash_table_unref(handler->current_headers);
  g_hash_table_unref(handler->callbacks);

  free(handler);
}

char *protocol_handler_cmd(struct protocol_handler *handler)
{
  return handler->current_command->str;
}

char *protocol_handler_header_value(struct protocol_handler *handler, const char *header_key)
{
  return (char *)g_hash_table_lookup(handler->current_headers, header_key);
}

struct callback_entry {
  protocol_handler_cb_t cb;
  void *data;
};

int protocol_handler_add_callback(struct protocol_handler *handler, const char *cmd, protocol_handler_cb_t cb, void *data)
{
  struct callback_entry *entry;

  entry = (struct callback_entry *)malloc(sizeof(struct callback_entry));
  entry->cb = cb;
  entry->data = data;
  
  g_hash_table_insert(handler->callbacks, (gpointer)cmd, entry);

  return 0;
}

static void protocol_handler_call_callback(struct protocol_handler *h)
{
  struct callback_entry *entry = (struct callback_entry *)g_hash_table_lookup(h->callbacks, h->current_command->str);

  if (entry != NULL)
    (*entry->cb)(h, entry->data);
}

static void protocol_error(struct protocol_handler *h, char c)
{
  fprintf(stderr, "Error in protocol: state %d char %c\n", h->state, c);
}

static void protocol_handler_end_of_header(struct protocol_handler *h)
{
  g_hash_table_insert(h->current_headers, g_strdup(h->current_header_key->str), g_strdup(h->current_header_value->str));
  g_string_truncate(h->current_header_key, 0);
  g_string_truncate(h->current_header_value, 0);
}

static void GH_print_func(gpointer key, gpointer value, gpointer user_data)
{
  fprintf(stderr, "Header: key=%s value=\"%s\"\n", (char *)key, (char *)value);
}

static void protocol_handler_end_of_command(struct protocol_handler *h)
{
#ifdef DEBUG
  fprintf(stderr, "Command: %s\n", h->current_command->str);
  g_hash_table_foreach (h->current_headers, GH_print_func, NULL);
  fprintf(stderr, "\n");
#endif

  protocol_handler_call_callback(h);

  g_string_truncate(h->current_command, 0);
  g_hash_table_remove_all(h->current_headers);
  g_string_truncate(h->current_header_key, 0);
  g_string_truncate(h->current_header_value, 0);
}

static int protocol_handler_input_char(struct protocol_handler *h, char c)
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

int protocol_handler_input(struct protocol_handler *handler)
{
  char c;

  while((c = fgetc(handler->input)) != EOF)
    protocol_handler_input_char(handler, c);
}

int protocol_handler_output_message(struct protocol_handler *handler, const char *cmd, ...)
{
  va_list ap;
  char *header_key, *header_value;

  fprintf(handler->output, "%s\n", cmd);

  va_start(ap, cmd);
  do {
    header_key = va_arg(ap, char *);
    if (header_key != NULL) {
      header_value = va_arg(ap, char *);
      fprintf(handler->output, "%s: %s\n", header_key, header_value);
    }
  } while(header_key != NULL);
  va_end(ap);

  fprintf(handler->output, "\n");
  fflush(handler->output);

  return 0;
}

