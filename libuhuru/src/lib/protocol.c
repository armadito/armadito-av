#include "libuhuru-config.h"
#include "protocol.h"

#include <glib.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

enum protocol_handler_state {
  EXPECTING_MSG,
  IN_MSG,
  EXPECTING_HEADER,
  IN_HEADER,
  EXPECTING_VALUE,
  IN_VALUE,
};

#define RECEIVE_BUFFER_LEN 1024

struct protocol_handler {
  int input_fd;
  FILE *output;
  enum protocol_handler_state state;
  GString *current_msg;
  GString *current_header_key, *current_header_value;
  GHashTable *current_headers;
  GHashTable *callbacks;
  char *receive_buffer;
};

static void str_free(gpointer p)
{
  g_free(p);
}

struct protocol_handler *protocol_handler_new(int input_fd, int output_fd)
{
  struct protocol_handler *h;

  h = (struct protocol_handler *)malloc(sizeof(struct protocol_handler));

  h->input_fd = input_fd;
  h->output = fdopen(output_fd, "w");
  if (h->output == NULL) {
    perror("fdopen");
    free(h);
    return NULL;
  }
  h->state = EXPECTING_MSG;
  h->current_msg = g_string_new("");
  h->current_header_key = g_string_new("");
  h->current_header_value = g_string_new("");
  h->current_headers = g_hash_table_new_full(g_str_hash, g_str_equal, str_free, str_free);

  h->callbacks = g_hash_table_new(g_str_hash, g_str_equal);

  h->receive_buffer = (char *)malloc(RECEIVE_BUFFER_LEN);

  return h;
}

void protocol_handler_free(struct protocol_handler *handler)
{
  g_string_free(handler->current_msg, TRUE);
  g_string_free(handler->current_header_key, TRUE);
  g_string_free(handler->current_header_value, TRUE);

  g_hash_table_unref(handler->current_headers);
  g_hash_table_unref(handler->callbacks);

  free(handler->receive_buffer);

  free(handler);
}

char *protocol_handler_get_msg(struct protocol_handler *handler)
{
  return handler->current_msg->str;
}

char *protocol_handler_get_header(struct protocol_handler *handler, const char *key)
{
  return (char *)g_hash_table_lookup(handler->current_headers, key);
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
  struct callback_entry *entry = (struct callback_entry *)g_hash_table_lookup(h->callbacks, h->current_msg->str);

  if (entry != NULL)
    (*entry->cb)(h, entry->data);
}

static void protocol_error(struct protocol_handler *h, char c)
{
  g_log(NULL, G_LOG_LEVEL_ERROR, "Error in protocol: state %d char %c", h->state, c);
}

static void protocol_handler_end_of_header(struct protocol_handler *h)
{
  g_hash_table_insert(h->current_headers, g_strdup(h->current_header_key->str), g_strdup(h->current_header_value->str));
  g_string_truncate(h->current_header_key, 0);
  g_string_truncate(h->current_header_value, 0);
}

static void GH_print_func(gpointer key, gpointer value, gpointer user_data)
{
  g_log(NULL, G_LOG_LEVEL_DEBUG, "Header: key=%s value=\"%s\"", (char *)key, (char *)value);
}

static void protocol_handler_end_of_msg(struct protocol_handler *h)
{
#if 0
#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "Msg: %s\n", h->current_msg->str);
  g_hash_table_foreach (h->current_headers, GH_print_func, NULL);
#endif
#endif

  protocol_handler_call_callback(h);

  g_string_truncate(h->current_msg, 0);
  g_hash_table_remove_all(h->current_headers);
  g_string_truncate(h->current_header_key, 0);
  g_string_truncate(h->current_header_value, 0);
}

static int protocol_handler_input_char(struct protocol_handler *h, char c)
{
  switch(h->state) {
  case EXPECTING_MSG:
    if (isupper(c) || c == '_')
      g_string_append_c(h->current_msg, c);
    else if (c == '\n')
      h->state = EXPECTING_HEADER;
    else
      protocol_error(h, c);
    break;
  case EXPECTING_HEADER:
    if (c == '\n') {
      protocol_handler_end_of_msg(h);
      h->state = EXPECTING_MSG;
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

int protocol_handler_receive(struct protocol_handler *handler)
{
  int n_read, i;

  n_read = read(handler->input_fd, handler->receive_buffer, RECEIVE_BUFFER_LEN);

  if (n_read == -1) {
    g_log(NULL, G_LOG_LEVEL_ERROR, "error in protocol_handler_receive: %s", strerror(errno));
  }

  if (n_read <= 0)
    return -1;

  for(i = 0; i < n_read; i++)
    protocol_handler_input_char(handler, handler->receive_buffer[i]);

  return 0;
}

int protocol_handler_send_msg(struct protocol_handler *handler, const char *msg, ...)
{
  va_list ap;
  char *header_key, *header_value;

  fprintf(handler->output, "%s\n", msg);

  va_start(ap, msg);
  do {
    header_key = va_arg(ap, char *);
    if (header_key != NULL) {
      header_value = va_arg(ap, char *);
      if (header_value != NULL)
	fprintf(handler->output, "%s: %s\n", header_key, header_value);
    }
  } while(header_key != NULL);
  va_end(ap);

  fprintf(handler->output, "\n");
  fflush(handler->output);

  return 0;
}

