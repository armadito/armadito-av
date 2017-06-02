/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#include <libarmadito/armadito.h>
#include <libjrpc/jrpc.h>

#include "core/event.h"
#include "core/handle.h"
#include "core/info.h"
#include "core/ondemand.h"
#include "rpc/rpctypes.h"

#include <glib.h>

struct scan_event_data {
	struct jrpc_connection *conn;
	struct a6o_on_demand *on_demand;
};

static void scan_event_cb(struct a6o_event *ev, void *data)
{
	struct scan_event_data *ev_data = (struct scan_event_data *)data;
	json_t *j_ev;
	time_t expected_scan_id = a6o_on_demand_get_id(ev_data->on_demand);
	int ret;

	switch(ev->type) {
	case EVENT_DETECTION:
		if (ev->u.ev_detection.scan_id != expected_scan_id)
			return;
		break;
	case EVENT_ON_DEMAND_COMPLETED:
		if (ev->u.ev_on_demand_completed.scan_id != expected_scan_id)
			return;
		break;
	case EVENT_ON_DEMAND_PROGRESS:
		if (ev->u.ev_on_demand_progress.scan_id != expected_scan_id)
			return;
		break;
	}

	if ((ret = JRPC_STRUCT2JSON(a6o_event, ev, &j_ev)))
		return;

	jrpc_notify(ev_data->conn, "notify_event", j_ev);
}

static gpointer scan_thread_fun(gpointer data)
{
	struct a6o_on_demand *on_demand = (struct a6o_on_demand *)data;

	a6o_on_demand_run(on_demand);

	a6o_on_demand_free(on_demand);

	return NULL;
}

static int scan_method(struct jrpc_connection *conn, json_t *params, json_t **result)
{
	struct armadito *armadito = (struct armadito *)jrpc_connection_get_data(conn);
	int ret;
	int event_mask;
	struct a6o_rpc_scan_param *s_param;
	struct a6o_on_demand *on_demand;
	struct scan_event_data *ev_data;
	enum a6o_scan_flags flags = 0;

	if ((ret = JRPC_JSON2STRUCT(a6o_rpc_scan_param, params, &s_param)))
		return ret;

	a6o_log(A6O_LOG_SERVICE, A6O_LOG_LEVEL_DEBUG, "scan path %s id %ld", s_param->root_path, s_param->scan_id);

	if (s_param->threaded)
		flags |= A6O_SCAN_THREADED;
	if (s_param->recursive)
		flags |= A6O_SCAN_RECURSE;
	on_demand = a6o_on_demand_new(armadito, s_param->root_path, s_param->scan_id, flags, s_param->send_progress);

	event_mask = EVENT_DETECTION | EVENT_ON_DEMAND_COMPLETED;
	if (s_param->send_progress)
		event_mask |= EVENT_ON_DEMAND_PROGRESS;

	ev_data = malloc(sizeof(struct scan_event_data));
	ev_data->conn = conn;
	ev_data->on_demand = on_demand;

	a6o_event_source_add_cb(a6o_get_event_source(armadito), event_mask, scan_event_cb, ev_data);

	g_thread_new("scan thread", scan_thread_fun, on_demand);

	return JRPC_OK;
}

static int status_method(struct jrpc_connection *conn, json_t *params, json_t **result)
{
	struct armadito *armadito = (struct armadito *)jrpc_connection_get_data(conn);
	int ret;
	struct a6o_info *info;

	info = a6o_info_new(armadito);

	if ((ret = JRPC_STRUCT2JSON(a6o_info, info, result)))
		return ret;

	a6o_info_free(info);

	return JRPC_OK;
}

static void listen_event_cb(struct a6o_event *ev, void *data)
{
	struct jrpc_connection *conn = (struct jrpc_connection *)data;
	json_t *j_ev;
	int ret;

	if ((ret = JRPC_STRUCT2JSON(a6o_event, ev, &j_ev)))
		return;

	jrpc_notify(conn, "notify_event", j_ev);
}

static int listen_method(struct jrpc_connection *conn, json_t *params, json_t **result)
{
	struct armadito *armadito = (struct armadito *)jrpc_connection_get_data(conn);
	struct a6o_rpc_listen_param *l_param;
	int event_mask = 0;
	int ret;

	if ((ret = JRPC_JSON2STRUCT(a6o_rpc_listen_param, params, &l_param)))
		return ret;

	if (l_param->detection)
		event_mask |= EVENT_DETECTION;
	if (l_param->on_demand)
		event_mask |= (EVENT_ON_DEMAND_START | EVENT_ON_DEMAND_COMPLETED);
	if (l_param->on_demand_progress)
		event_mask |= EVENT_ON_DEMAND_PROGRESS;
	if (l_param->quarantine)
		event_mask |= EVENT_QUARANTINE;
	if (l_param->real_time_prot)
		event_mask |= EVENT_REAL_TIME_PROT;
	if (l_param->av_update)
		event_mask |= EVENT_AV_UPDATE;

	a6o_event_source_add_cb(a6o_get_event_source(armadito),  event_mask, listen_event_cb, conn);

	return JRPC_OK;
}

static struct jrpc_mapper *rpcbe_mapper;

static void create_rpcbe_mapper(void)
{
	rpcbe_mapper = jrpc_mapper_new();
	jrpc_mapper_add(rpcbe_mapper, "scan", scan_method);
	jrpc_mapper_add(rpcbe_mapper, "status", status_method);
	jrpc_mapper_add(rpcbe_mapper, "listen", listen_method);
}

struct jrpc_mapper *a6o_get_rpcbe_mapper(void)
{
	if (rpcbe_mapper == NULL)
		create_rpcbe_mapper();

	return rpcbe_mapper;
}
