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

static void debug_args(struct ipc_manager *m)
{
	int i;
	struct ipc_value *argv;

	fprintf(stderr, "tag %d argc %d", ipc_manager_get_msg_id(m), ipc_manager_get_argc(m));

	argv = ipc_manager_get_argv(m);
	for (i = 0; i < ipc_manager_get_argc(m); i++) {
		switch(argv[i].type) {
		case IPC_INT32_T:
			fprintf(stderr, " [%d] = (int32)%d", i, argv[i].value.v_int32);
			break;
		case IPC_STRING_T:
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
	ipc_int32_t i;

	fprintf(stderr, "handler 1: ");
	debug_args(manager);
	fprintf(stderr, "\n");
	ipc_manager_get_arg_at(manager, 0, IPC_STRING_T, &ps);
	fprintf(stderr, "arg 0 %s\n", ps);
	ipc_manager_get_arg_at(manager, 1, IPC_INT32_T, &i);
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

	ipc = ipc_manager_new(0);

	ipc_manager_add_handler(ipc, IPC_MSG_ID_SCAN, handler1, NULL);
	ipc_manager_add_handler(ipc, IPC_MSG_ID_SCAN_FILE, handler2, NULL);

	while (ipc_manager_receive(ipc) > 0)
		;

	return 0;
}
