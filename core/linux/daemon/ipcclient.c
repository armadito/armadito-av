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

#include <libarmadito.h>

#include <libarmadito-config.h>

#include "ipc.h"

#include "ipcclient.h"

#include <errno.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct ipc_client {
	struct armadito *armadito;
	int sock;
	struct ipc_manager *manager;
	GMutex lock;
};

static void ipc_client_close(struct ipc_client *cl);

static void ipc_ping_handler(struct ipc_manager *m, void *data)
{
	struct ipc_client *cl = (struct ipc_client *)data;

#ifdef DEBUG
	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "ipc: ping handler called");
#endif

	ipc_manager_msg_send(cl->manager, IPC_MSG_ID_PONG, IPC_NONE_T);
}

static void scan_callback(struct a6o_report *report, void *callback_data)
{
	struct ipc_client *cl = (struct ipc_client *)callback_data;

	g_mutex_lock(&cl->lock);
	ipc_manager_msg_send(cl->manager,
			IPC_MSG_ID_SCAN_FILE,
			IPC_STRING_T, report->path,
			IPC_STRING_T, a6o_file_status_str(report->status),
			IPC_STRING_T, report->mod_name,
			IPC_STRING_T, report->mod_report,
			IPC_STRING_T, a6o_action_pretty_str(report->action),
			IPC_INT32_T, report->progress,
			IPC_NONE_T);
	g_mutex_unlock(&cl->lock);
}

static void ipc_scan_handler(struct ipc_manager *m, void *data)
{
	struct ipc_client *cl = (struct ipc_client *)data;
	char *path;
	int threaded;
	int recurse;
	enum a6o_scan_flags flags = 0;
	struct a6o_on_demand *on_demand;

#ifdef DEBUG
	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "ipc: scan handler called");
#endif

	ipc_manager_get_arg_at(m, 0, IPC_STRING_T, &path);
	ipc_manager_get_arg_at(m, 1, IPC_INT32_T, &threaded);
	ipc_manager_get_arg_at(m, 2, IPC_INT32_T, &recurse);

	if (threaded)
		flags |= ARMADITO_SCAN_THREADED;

	if (recurse)
		flags |= ARMADITO_SCAN_RECURSE;

	on_demand = a6o_on_demand_new(cl->armadito, 0, path, flags);

	a6o_scan_add_callback(a6o_on_demand_get_scan(on_demand), scan_callback, cl);

	a6o_on_demand_run(on_demand);

	a6o_on_demand_free(on_demand);

	ipc_manager_msg_send(cl->manager, IPC_MSG_ID_SCAN_END, IPC_NONE_T);

	ipc_client_close(cl);

#ifdef DEBUG
	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "ipc: scan handler finished");
#endif
}

static void info_send(struct ipc_manager *manager, struct a6o_info *info)
{
	struct a6o_module_info **m;

	for(m = info->module_infos; *m != NULL; m++) {
		ipc_manager_msg_begin(manager, IPC_MSG_ID_INFO_MODULE);
		ipc_manager_msg_add(manager,
				IPC_STRING_T, (*m)->name,
				IPC_STRING_T, a6o_update_status_str((*m)->mod_status),
				IPC_STRING_T, (*m)->update_date,
				IPC_NONE_T);

		if ((*m)->base_infos != NULL) {
			struct a6o_base_info **b;

			for(b = (*m)->base_infos; *b != NULL; b++)
				ipc_manager_msg_add(manager,
						IPC_STRING_T, (*b)->name,
						IPC_STRING_T, (*b)->date,
						IPC_STRING_T, (*b)->version,
						IPC_INT32_T, (*b)->signature_count,
						IPC_STRING_T, (*b)->full_path,
						IPC_NONE_T);
		}

		ipc_manager_msg_end(manager);
	}

	ipc_manager_msg_send(manager,
			IPC_MSG_ID_INFO_END,
			IPC_STRING_T, a6o_update_status_str(info->global_status),
			IPC_NONE_T);
}

static void ipc_info_handler(struct ipc_manager *manager, void *data)
{
	struct ipc_client *cl = (struct ipc_client *)data;
	struct a6o_info *info;

#ifdef DEBUG
	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "ipc: info handler called");
#endif

	info = a6o_info_new(cl->armadito);

	info_send(manager, info);

	a6o_info_free(info);

	ipc_client_close(cl);

#ifdef DEBUG
	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "ipc: info handler finished");
#endif
}

struct ipc_client *ipc_client_new(int sock, struct armadito *armadito)
{
	struct ipc_client *cl = (struct ipc_client *)malloc(sizeof(struct ipc_client));

	cl->armadito = armadito;
	cl->sock = sock;
	cl->manager = ipc_manager_new(cl->sock);

	ipc_manager_add_handler(cl->manager, IPC_MSG_ID_PING, ipc_ping_handler, cl);
	ipc_manager_add_handler(cl->manager, IPC_MSG_ID_SCAN, ipc_scan_handler, cl);
	ipc_manager_add_handler(cl->manager, IPC_MSG_ID_INFO, ipc_info_handler, cl);

	g_mutex_init(&cl->lock);

	return cl;
}

static void ipc_client_close(struct ipc_client *cl)
{
	if (cl->sock < 0)
		return;

	if (close(cl->sock) < 0) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "closing socket %d failed (%s)", cl->sock, strerror(errno));
	}

	cl->sock = -1;
}

void ipc_client_free(struct ipc_client *cl)
{
	ipc_manager_free(cl->manager);

	g_mutex_clear(&cl->lock);

	ipc_client_close(cl);

	free(cl);
}

int ipc_client_process(struct ipc_client *cl)
{
	int ret;

	if (cl->sock < 0)
		return 0;

	ret = ipc_manager_receive(cl->manager);

	return ret;
}
