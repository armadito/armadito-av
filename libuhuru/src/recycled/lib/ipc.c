#include "libuhuru-config.h"

#include <libuhuru/ipc.h>
#include "os/io.h"
#include "os/string.h"

#include <assert.h>
#include <glib.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_INPUT_BUFFER_SIZE 1024

/* structure containing a type ID and an union for the various argument types */
struct ipc_arg {
  ipc_type_id_t type;       /* type ID          */
  union {
    ipc_int32_t v_int32;    /* 32 bits integer; valid when type == IPC_TYPE_INT32 */
    char *v_str;            /* null terminated string; valid when type == IPC_TYPE_STRING */
  } value;
};

/*
  the state of the finite state automaton for protocol decoding
 */
enum ipc_manager_state {
  STATE_EXPECTING_MSG_ID = 1,
  STATE_EXPECTING_TYPE_ID,
  STATE_ARG_INT32,
  STATE_ARG_STRING,
};

/*
  structure to store a handler function and its (void *) argument
 */
struct ipc_handler_entry {
  ipc_handler_t handler;
  void *data;
};

struct ipc_manager {
  void *g_handle;                      /* generic handler for I/O: (int fd for Unix, HANDLE hFile for Windows) */
  ipc_write_fun_t write_fun;           /* write I/O function */
  ipc_read_fun_t read_fun;             /* read I/O function */
  size_t input_buffer_size;
  char *input_buffer;
  enum ipc_manager_state state;        /* decoding automaton current state */
  ipc_msg_id_t msg_id;                 /* current message ID */
  union {
    ipc_int32_t i;
    unsigned char t[sizeof(ipc_int32_t)]; /* this trick allows to transform 4 bytes into one 32 bits integer without shifts and adds  */
  } int32_arg;                         /* current int32 argument */
  GString *str_arg;                    /* current string argument */
  GArray *argv;                        /* variable size array of arguments of current message, i.e. of struct ipc_arg */
  struct ipc_handler_entry *handlers;  /* array of handlers */
};

struct ipc_manager *ipc_manager_new(void *g_handle, ipc_write_fun_t write_fun, ipc_read_fun_t read_fun);
{
  struct ipc_manager *m = g_new(struct ipc_manager, 1);

  m->g_handle = g_handle;
  m->write_fun = write_fun;
  m->read_fun = read_fun;

  m->input_buffer_size = DEFAULT_INPUT_BUFFER_SIZE;
  m->input_buffer = (char *)malloc(m->input_buffer_size);

  m->state = EXPECTING_MSG_ID;

  m->int32_arg.i = 0;
  m->str_arg = g_string_new("");

  /* argv is a variable size array of 'ipc_arg' structures */
  m->argv = g_array_new(FALSE, FALSE, sizeof(struct ipc_arg));

  m->handlers = g_new0(struct ipc_handler_entry, IPC_MSG_ID_LAST - IPC_MSG_ID_FIRST + 1);

  return m;
}

void ipc_manager_free(struct ipc_manager *manager)
{
  free(manager->input_buffer);
  g_string_free(manager->str_arg, TRUE);
}

ipc_msg_id_t ipc_manager_get_msg_id(struct ipc_manager *manager)
{
  return manager->msg_id;
}

int ipc_manager_get_argc(struct ipc_manager *manager)
{
  return manager->argv->len;
}

#if 0
was never used
struct ipc_arg *ipc_manager_get_argv(struct ipc_manager *manager)
{
  return (struct ipc_arg *)manager->argv->data;
}
#endif

int ipc_manager_get_arg_at(struct ipc_manager *manager, int index, ipc_type_id_t type, void *pvalue)
{
  struct ipc_arg *argv;

  /* check index range */
  if (index >= ipc_manager_get_argc(manager)) {
    g_log(NULL, UHURU_LOG_LEVEL_ERROR, "IPC: argument index out of range %d >= %d ", index, ipc_manager_get_argc(manager));
    return -1;
  }

  /* check if wanted type matches type at 'index' in current arguments */
  argv = ipc_manager_get_argv(manager);
  if (argv[index].type != type) {
    g_log(NULL, UHURU_LOG_LEVEL_ERROR, "IPC: invalid argument type %d != %d ", type, argv[index].type);
    return -1;
  }

 /* ok: copy the values */
  switch(type) {
  case IPC_TYPE_INT32:
    *((ipc_int32_t *)pvalue) = argv[index].value.v_int32;
    break;
  case IPC_TYPE_STRING:
    /* note that the string is not strdup'ed */
    *((char **)pvalue) = argv[index].value.v_str;
    break;
  }

  return 0;
}

int ipc_manager_add_handler(struct ipc_manager *manager, ipc_msg_id_t msg_id, ipc_handler_t handler, void *data)
{
  /* check msg_id in range */
  if (msg_id < IPC_MSG_ID_FIRST || msg_id > IPC_MSG_ID_LAST) {
    g_log(NULL, UHURU_LOG_LEVEL_ERROR, "IPC: cannot add handler for msg_id %d: out of range %d - %d ", msg_id, IPC_MSG_ID_FIRST, IPC_MSG_ID_LAST);
    return -1;
  }

  /* is handler already set for this message id? yes => error */
  if (manager->handlers[msg_id].handler != NULL) {
    g_log(NULL, UHURU_LOG_LEVEL_ERROR, "IPC: cannot add handler for msg_id %d: handler already set", msg_id);
    return -1;
  }

  manager->handlers[msg_id].handler = handler;
  manager->handlers[msg_id].data = data;

  return 0;
}

/* append an int32 argument to array of arguments */
static void ipc_manager_add_int32_arg(struct ipc_manager *m)
{
  struct ipc_arg v;

  v.type = IPC_TYPE_INT32;
  v.value.v_int32 = m->int32_arg.i;

  g_array_append_val(m->argv, v);
}

/* append a string argument to array of arguments */
static void ipc_manager_add_str_arg(struct ipc_manager *m)
{
  struct ipc_arg v;

  v.type = IPC_TYPE_STRING;
  /* str_arg is a GString; its content must be strdup'ed because decoding next string argument will overwrite it */
  v.value.v_str = os_strdup(m->str_arg->str);

  g_array_append_val(m->argv, v);
}

#ifdef DEBUG
static void ipc_manager_debug(struct ipc_manager *m)
{
  int i;
  struct ipc_arg *argv;

  g_log(NULL, UHURU_LOG_LEVEL_DEBUG, "IPC: msg_id %d argc %d", m->msg_id, ipc_manager_get_argc(m));

  argv = ipc_manager_get_argv(m);
  for (i = 0; i < ipc_manager_get_argc(m); i++) {
    switch(argv[i].type) {
    case IPC_TYPE_INT32:
      g_log(NULL, UHURU_LOG_LEVEL_DEBUG, "IPC: arg[%d] = (int32)%d", i, argv[i].value.v_int32);
      break;
    case IPC_TYPE_STRING:
      g_log(NULL, UHURU_LOG_LEVEL_DEBUG, "IPC: arg[%d] = (char *)%s", i, argv[i].value.v_str);
      break;
    default:
      g_log(NULL, UHURU_LOG_LEVEL_DEBUG, "IPC: arg[%d] = ???", i);
      break;
    }
  }
}
#endif

/* call the handler associated with current message ID; this function is called after decoding a complete message */
static void ipc_manager_call_handler(struct ipc_manager *m)
{
  ipc_handler_t handler;
  void *data;

  /* check range of message ID; should probably be done inside decoding automaton */
  if (m->msg_id < IPC_MSG_ID_FIRST || m->msg_id > IPC_MSG_ID_LAST) {
    g_log(NULL, UHURU_LOG_LEVEL_WARNING, "IPC: received msg_id %d out of range %d - %d ", m->msg_id, IPC_MSG_ID_FIRST, IPC_MSG_ID_LAST);
    return;
  }

  /* get the handler; it may be NULL */
  handler = m->handlers[m->msg_id].handler;
  if (handler == NULL)
    return;

  /* call it, with the argument 'data' that was passed to ipc_manager_add_handler() */
  data = m->handlers[m->msg_id].data;
  (*handler)(m, data);
}

/* called by the decoding automaton when receiving an END_OF_MESSAGE marker */
static void ipc_manager_end_of_msg(struct ipc_manager *m)
{
  int i;
  struct ipc_arg *argv;

#ifdef DEBUG
  ipc_manager_debug(m);
#endif

  /* first, process the message by calling handlers */
  ipc_manager_call_handler(m);

  /* free the arguments, but only the IPC_TYPE_STRING of course, which were strdup'ed in ipc_manager_add_str_arg() */
  argv = ipc_manager_get_argv(m);
  for (i = 0; i < ipc_manager_get_argc(m); i++) {
    if (argv[i].type == IPC_TYPE_STRING) {
      free(argv[i].value.v_str);
      argv[i].value.v_str = NULL;
    }
  }

  /* resize the array to 0 for next message */
  g_array_set_size(m->argv, 0);
}

/* the protocol decoding automaton */


INITIAL -> STATE_EXPECTING_MSG_ID  ->



STATE_EXPECTING_TYPE_ID
STATE_ARG_INT32
STATE_ARG_STRING



static void ipc_manager_input_char(struct ipc_manager *m, unsigned char c)
{
  switch(m->state) {
  case STATE_EXPECTING_MSG_ID:
    m->msg_id = c;
    m->state = STATE_EXPECTING_ARG;
    break;
  case STATE_EXPECTING_ARG:
    switch(c) {
    case IPC_TYPE_INT32:
      m->received_count = 0;
      m->state = STATE_ARG_INT32;
      break;
    case IPC_TYPE_STRING:
      m->state = STATE_ARG_STRING;
      break;
    case IPC_END_OF_MESSAGE:
      /* got END_OF_MESSAGE marker; process current message */
      ipc_manager_end_of_msg(m);
      /* and restart decoding */
      m->state = STATE_EXPECTING_MSG_ID;
      break;
    default:
      g_log(NULL, UHURU_LOG_LEVEL_ERROR, "error in ipc_manager_receive: invalid type msg_id %c %d", c, c);
      break;
    }
    break;
  case STATE_ARG_INT32:
    m->int32_arg.t[m->received_count++] = c;
    if (m->received_count == sizeof(ipc_int32_t)) {
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

  n_read = os_read(manager->io_fd, manager->input_buffer, manager->input_buffer_size);

  if (n_read == -1) {
    g_log(NULL, UHURU_LOG_LEVEL_ERROR, "error in ipc_manager_receive: %s", os_strerror(errno));
  }

  if (n_read < 0)
    return -1;
  else if (n_read == 0)
    return 0;

  for(i = 0; i < n_read; i++)
    ipc_manager_input_char(manager, manager->input_buffer[i]);

  return 1;
}

static size_t ipc_manager_write(struct ipc_manager *manager, char *buffer, size_t len)
{
  size_t to_write = len;

  assert(len > 0);

  while (to_write > 0) {
    int w = os_write(manager->io_fd, buffer, to_write);

    if (w < 0) {
      g_log(NULL, UHURU_LOG_LEVEL_ERROR, "error in ipc_manager_write_buffer: %s", os_strerror(errno));
      return -1;
    }

    if (w == 0)
      return 0;

    buffer += w;
    to_write -= w;
  }

  return len;
}

int ipc_manager_msg_begin(struct ipc_manager *manager, ipc_msg_id_t msg_id)
{
  return ipc_manager_write(manager, &msg_id, sizeof(guchar));
}

static ipc_manager_msg_addv(struct ipc_manager *manager, va_list ap)
{
  ipc_type_id_t type;

  do {
    ipc_int32_t v_int32;
    char *v_str;
    int i_type = va_arg(ap, int);

    type = (ipc_type_id_t)(i_type & 0xff);

    if (type != IPC_NONE_T) {
      ipc_manager_write(manager, &type, sizeof(ipc_type_id_t));
      switch(type) {
      case IPC_TYPE_INT32:
	v_int32 = va_arg(ap, ipc_int32_t);
	ipc_manager_write(manager, (char *)&v_int32, sizeof(ipc_int32_t));
	break;
      case IPC_TYPE_STRING:
	v_str = va_arg(ap, char *);
	if (v_str != NULL)
	  ipc_manager_write(manager, v_str, strlen(v_str) + 1);
	else
	  ipc_manager_write(manager, "", sizeof(char));
	break;
      }
    }
  } while (type != IPC_NONE_T);
}

int ipc_manager_msg_add(struct ipc_manager *manager, ...)
{
  va_list ap;

  va_start(ap, manager);

  ipc_manager_msg_addv(manager, ap);

  va_end(ap);

  return 0;
}

int ipc_manager_msg_end(struct ipc_manager *manager)
{
  ipc_type_id_t type = IPC_NONE_T;

  ipc_manager_write(manager, &type, sizeof(ipc_type_id_t));

  return 0;
}

int ipc_manager_msg_send(struct ipc_manager *manager, ipc_msg_id_t msg_id, ...)
{
  va_list ap;

  /* this function just calls the above functions after initializing the va_list  */

  va_start(ap, msg_id);

  ipc_manager_msg_begin(manager, msg_id);

  ipc_manager_msg_addv(manager, ap);

  ipc_manager_msg_end(manager);

  va_end(ap);

  return 0;
}
