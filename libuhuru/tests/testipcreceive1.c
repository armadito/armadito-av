#include "lib/ipc.h"

#include <assert.h>
#include <stdio.h>

static void debug_args(struct ipc_manager *m)
{
  int i;
  struct ipc_value *argv;

  fprintf(stderr, "tag %d argc %d", ipc_manager_get_msg_id(m), ipc_manager_get_argc(m));

  argv = ipc_manager_get_argv(m);
  for (i = 0; i < ipc_manager_get_argc(m); i++) {
    switch(argv[i].type) {
    case IPC_INT32:
      fprintf(stderr, " [%d] = (int32)%d", i, argv[i].value.v_int32);
      break;
    case IPC_STRING:
      fprintf(stderr, " [%d] = (char *)%s", i, argv[i].value.v_str);
      break;
    default:
      fprintf(stderr, " [%d] = ???", i);
      break;
    }
  }
}

static void handler1(struct ipc_manager *manager, void *data)
{
  char *ps;
  gint32 i;

  fprintf(stderr, "handler 1: ");
  debug_args(manager);
  fprintf(stderr, "\n");
  ipc_manager_get_arg_at(manager, 0, IPC_STRING, &ps);
  fprintf(stderr, "arg 0 %s\n", ps);
  ipc_manager_get_arg_at(manager, 1, IPC_INT32, &i);
  fprintf(stderr, "arg 1 %d\n", i);
}

static void handler2(struct ipc_manager *manager, void *data)
{
  fprintf(stderr, "handler 2: ");
  debug_args(manager);
  fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
  struct ipc_manager *ipc;

  ipc = ipc_manager_new(0, 1);

  ipc_manager_add_handler(ipc, IPC_MSG_ID_SCAN, handler1, NULL);
  ipc_manager_add_handler(ipc, IPC_MSG_ID_SCAN_FILE, handler2, NULL);

  while (ipc_manager_receive(ipc) > 0)
    ;

  return 0;
}
