/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

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
