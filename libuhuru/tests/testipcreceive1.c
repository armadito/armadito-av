#include "lib/ipc.h"

#include <assert.h>
#include <stdio.h>

#define IPC_MSG_SCAN_FILE     33
#define IPC_MSG_SCAN_START    34

int main(int argc, char **argv)
{
  struct ipc_manager *ipc;

  ipc = ipc_manager_new(0, 1);

  while (ipc_manager_receive(ipc) > 0)
    ;

  return 0;
}
