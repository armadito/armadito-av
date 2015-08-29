#include "libuhuru-config.h"
#include "ipc.h"

#include <glib.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum ipc_manager_state {
  EXPECTING_TAG = 1,
  EXPECTING_ARG,
  IN_ARG_INT32,
  IN_ARG_STRING,
};

#define DEFAULT_INPUT_BUFFER_SIZE 1024

struct ipc_manager {
  int input_fd;
  FILE *output;
  size_t input_buffer_size;
  char *input_buffer;
  enum ipc_manager_state state;
  guchar tag;
  int received_count;
  union {
    gint32 i;
    guchar t[sizeof(gint32)];
  } int32_arg;
  GString *str_arg;
  GArray *argv;
};

struct ipc_manager *ipc_manager_new(int input_fd, int output_fd)
{
  struct ipc_manager *m = g_new(struct ipc_manager, 1);

  m->input_fd = input_fd;
  m->output = fdopen(output_fd, "w");
  if (m->output == NULL) {
    perror("fdopen");
    free(m);
    return NULL;
  }

  m->input_buffer_size = DEFAULT_INPUT_BUFFER_SIZE;
  m->input_buffer = (char *)malloc(m->input_buffer_size);

  m->state = EXPECTING_TAG;

  m->int32_arg.i = 0;
  m->str_arg = g_string_new("");

  m->argv = g_array_new(FALSE, FALSE, sizeof(struct ipc_value));

  return m;
}

void ipc_manager_free(struct ipc_manager *manager)
{
  free(manager->input_buffer);
  g_string_free(manager->str_arg, TRUE);
  /* FIXME: to be completed */
}

int ipc_manager_get_argc(struct ipc_manager *manager)
{
  return manager->argv->len;
}

struct ipc_value *ipc_manager_get_argv(struct ipc_manager *manager)
{
  return (struct ipc_value *)manager->argv->data;
}

int ipc_manager_add_callback(struct ipc_manager *manager, ipc_tag_t tag, ipc_handler_t cb, void *data)
{
  return 0;
}

static void ipc_manager_add_int32_arg(struct ipc_manager *m)
{
  struct ipc_value v;

  v.type = IPC_INT32;
  v.value.v_int32 = m->int32_arg.i;

  g_array_append_val(m->argv, v);
}

static void ipc_manager_add_str_arg(struct ipc_manager *m)
{
  struct ipc_value v;

  v.type = IPC_STRING;
  v.value.v_str = strdup(m->str_arg->str);

  g_array_append_val(m->argv, v);
}

#ifdef DEBUG
static void ipc_manager_debug(struct ipc_manager *m)
{
  int i;
  struct ipc_value *argv;

  g_log(NULL, G_LOG_LEVEL_DEBUG, "IPC: tag %d argc %d", m->tag, ipc_manager_get_argc(m));

  argv = ipc_manager_get_argv(m);
  for (i = 0; i < ipc_manager_get_argc(m); i++) {
    switch(argv[i].type) {
    case IPC_INT32:
      g_log(NULL, G_LOG_LEVEL_DEBUG, "IPC: arg[%d] = (int32)%d", i, argv[i].value.v_int32);
      break;
    case IPC_STRING:
      g_log(NULL, G_LOG_LEVEL_DEBUG, "IPC: arg[%d] = (char *)%s", i, argv[i].value.v_str);
      break;
    default:
      g_log(NULL, G_LOG_LEVEL_DEBUG, "IPC: arg[%d] = ???", i);
      break;
    }
  }
}
#endif  

static void ipc_manager_end_of_msg(struct ipc_manager *m)
{
#ifdef DEBUG
  ipc_manager_debug(m);
#endif

  /* FIXME: must free strings */
  g_array_set_size(m->argv, 0);
}

static void ipc_manager_input_char(struct ipc_manager *m, guchar c)
{
#if 0
  g_log(NULL, G_LOG_LEVEL_DEBUG, "IPC: processing char %0d %hhd %c state %d", c, c, (c != '\0') ? c : NULL, m->state);
#endif

  switch(m->state) {
  case EXPECTING_TAG:
    m->tag = c;
    m->state = EXPECTING_ARG;
    break;
  case EXPECTING_ARG:
    switch(c) {
    case IPC_INT32:
      m->received_count = 0;
      m->state = IN_ARG_INT32;
      break;
    case IPC_STRING:
      m->state = IN_ARG_STRING;
      break;
    case IPC_NONE:
      m->state = EXPECTING_TAG;
      ipc_manager_end_of_msg(m);
      break;
    default:
      g_log(NULL, G_LOG_LEVEL_ERROR, "error in ipc_manager_receive: invalid type tag %c %d", c, c);
      break;
    }
    break;
  case IN_ARG_INT32:
    m->int32_arg.t[m->received_count++] = c;
    /* g_log(NULL, G_LOG_LEVEL_DEBUG, "received %d bytes of %d", m->received_count, sizeof(gint32)); */
    if (m->received_count == sizeof(gint32)) {
      ipc_manager_add_int32_arg(m);
      m->received_count = 0;
      m->state = EXPECTING_ARG;
    }
    break;
  case IN_ARG_STRING:
    if (c != '\0')
      g_string_append_c(m->str_arg, c);
    else {
      ipc_manager_add_str_arg(m);
      g_string_truncate(m->str_arg, 0);
      m->state = EXPECTING_ARG;
    }
    break;
  }
}

int ipc_manager_receive(struct ipc_manager *manager)
{
  int n_read, i;

  n_read = read(manager->input_fd, manager->input_buffer, manager->input_buffer_size);

  if (n_read == -1) {
    g_log(NULL, G_LOG_LEVEL_ERROR, "error in ipc_manager_receive: %s", strerror(errno));
  }

  if (n_read < 0)
    return -1;
  else if (n_read == 0)
    return 0;

  for(i = 0; i < n_read; i++)
    ipc_manager_input_char(manager, manager->input_buffer[i]);

  return 1;
}

int ipc_manager_send_msg(struct ipc_manager *manager, guchar tag, ...)
{
  va_list ap;
  int i_type;
  ipc_type_t type;
  gint32 v_int32;
  char *v_str;

  fwrite(&tag, sizeof(guchar), 1, manager->output);

  va_start(ap, tag);

  do {
    i_type = va_arg(ap, int);

    type = (ipc_type_t)(i_type & 0xff);
    fwrite(&type, sizeof(ipc_type_t), 1, manager->output);

    switch(type) {
    case IPC_INT32:
      v_int32 = va_arg(ap, gint32);
      fwrite(&v_int32, sizeof(gint32), 1, manager->output);
      break;
    case IPC_STRING:
      v_str = va_arg(ap, char *);
      fwrite(v_str, strlen(v_str) + 1, 1, manager->output);
      break;
    case IPC_NONE:
      break;
    }

  } while (type != IPC_NONE);

  va_end(ap);

  fflush(manager->output);

  return 0;
}
