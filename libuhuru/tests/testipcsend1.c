#include "lib/ipc.h"

#include <assert.h>
#include <stdio.h>

#define IPC_MSG_SCAN_FILE     33
#define IPC_MSG_SCAN_START    34

int main(int argc, char **argv)
{
  struct ipc_manager *ipc;

  ipc = ipc_manager_new(0, 1);

  ipc_manager_send_msg(ipc, 69, 
		       IPC_NONE);

  ipc_manager_send_msg(ipc, 69, 
		       IPC_STRING, "hello", 
		       IPC_NONE);

  ipc_manager_send_msg(ipc, 42, 
		       IPC_INT32, 1, 
		       IPC_STRING, "hello", 
		       IPC_INT32, -1, 
		       IPC_NONE);
  
  ipc_manager_send_msg(ipc, IPC_MSG_SCAN_FILE, 
		       IPC_STRING, "/var/tmp/foo/bar/zob2", 
		       IPC_STRING, "MALWARE",
		       IPC_STRING, "GrosTrojan",
		       IPC_STRING, "QUARANTINE",
		       IPC_NONE);

  ipc_manager_send_msg(ipc, IPC_MSG_SCAN_START, 
		       IPC_STRING, "/var/tmp/foo", 
		       IPC_INT32, 42,
		       IPC_NONE);

  return 0;
}
