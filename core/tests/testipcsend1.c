#include "daemon/ipc.h"

#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	struct ipc_manager *ipc;

	ipc = ipc_manager_new(1);

	ipc_manager_msg_send(ipc, IPC_MSG_ID_SCAN,
			IPC_STRING_T, "/var/tmp/foo",
			IPC_INT32_T, 42,
			IPC_NONE_T);

	ipc_manager_msg_send(ipc, IPC_MSG_ID_SCAN_FILE,
			IPC_STRING_T, "/var/tmp/foo/bar/zob2",
			IPC_STRING_T, "MALWARE",
			IPC_STRING_T, "GrosTrojan",
			IPC_STRING_T, "QUARANTINE",
			IPC_NONE_T);

	return 0;
}
