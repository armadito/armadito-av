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

#include <json.h>
#include "glib.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <libarmadito.h>

#include "api.h"
#include "httpd.h"

/* usefull??? */
typedef int64_t api_token_t;

#define API_TOKEN_HEADER "X-Armadito-Token"

#define JSON_400 "{\"code\":400, \"message\": \"Bad Request. Make sure your request has a X-Armadito-Token header\"}"
#define JSON_403 "{\"code\":403, \"message\": \"Request forbidden. Make sure your request has a User-Agent header\"}"
#define JSON_404 "{\"code\":404, \"message\": \"Not found\"}"
#define JSON_415 "{\"code\":415, \"message\": \"Unsupported Media Type. Content-Type must be application/json\"}"

struct api_handler {
	GHashTable *client_table;
	struct MHD_Response *response_400;
	struct MHD_Response *response_403;
	struct MHD_Response *response_404;
	struct MHD_Response *response_415;
};


static int api_handler_add_client(struct api_handler *a, int64_t token);
static struct api_client *api_handler_get_client(struct api_handler *a, int64_t token);
static int api_handler_remove_client(struct api_handler *a, int64_t token);

struct api_client {
	GAsyncQueue *event_queue;
};

static struct api_client *api_client_new(void)
{
	struct api_client *c = malloc(sizeof(struct api_client));

	c->event_queue = g_async_queue_new();

	return c;
}

static void api_client_destroy(struct api_client *c)
{
	g_async_queue_unref(c->event_queue);
}

typedef int (*api_cb_t)(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out);

static int token_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out);
static int ping_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out);
static int scan_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out);
static int poll_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out);

static struct api_dispatch_entry {
	const char *path;
	api_cb_t api_cb;
} api_dispatch_table[] = {
	{ "/token", token_api_cb},
	{ "/ping", ping_api_cb},
	{ "/scan", scan_api_cb},
	{ "/poll", poll_api_cb},
	{ NULL, NULL},
};

static api_cb_t get_api_cb(const char *path)
{
	struct api_dispatch_entry *p;

	for (p = api_dispatch_table; p->path != NULL && strcmp(p->path, path); p++)
		;

	if (p->path != NULL)
		return p->api_cb;

	return NULL;
}

static const char *api_get_user_agent(struct MHD_Connection *connection)
{
	return MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_USER_AGENT);
}

static const char *api_get_token(struct MHD_Connection *connection, int64_t *p_token)
{
	const char *s_token;

	s_token = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, API_TOKEN_HEADER);
	if (s_token != NULL)
		*p_token = atoll(s_token);

	return s_token;
}

#define HASH_ONE(H, C) (H) ^= ((H) << 5) + ((H) >> 2) + (C)
/* #define HASH_ONE(H, C) (H) = (((H) << 5) + (H)) + (C) */

static void hash_init(int64_t *hash)
{
	/* *hash = 5381;*/
	*hash = 0;
}

static void hash_buff(const char *buff, size_t len, int64_t *hash)
{
	for ( ; len--; buff++)
		HASH_ONE(*hash, *buff);
}

static void hash_str(const char *str, int64_t *hash)
{
	for ( ; *str; str++)
		HASH_ONE(*hash, *str);
}

static int token_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out)
{
	int64_t token;
	const char *user_agent;
	char here;
	time_t now;

	hash_init(&token);
	time(&now);
	hash_buff((const char *)&now, sizeof(time_t), &token);
	user_agent = api_get_user_agent(connection);
	hash_str(user_agent, &token);
	hash_buff(&here, sizeof(char *), &token);

	if (token < 0)
		token = -token;

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "token %lld", token);

	*out = json_object_new_object();
	json_object_object_add(*out, "token", json_object_new_int64(token));

	api_handler_add_client(a, token);

	return 0;
}

static int ping_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out)
{
	*out = json_object_new_object();
	json_object_object_add(*out, "status", json_object_new_string("ok"));

	return 0;
}

static struct json_object *report_json(struct a6o_report *report)
{
	struct json_object *j_report;

	j_report = json_object_new_object();

	json_object_object_add(j_report, "progress", json_object_new_int(report->progress));
	json_object_object_add(j_report, "malware_count", json_object_new_int(report->malware_count));
	json_object_object_add(j_report, "suspicious_count", json_object_new_int(report->suspicious_count));
	json_object_object_add(j_report, "scanned_count", json_object_new_int(report->scanned_count));

	if (report->path != NULL)
		json_object_object_add(j_report, "path", json_object_new_string(report->path));

	json_object_object_add(j_report, "scan_status", json_object_new_string(a6o_file_status_pretty_str(report->status)));
	json_object_object_add(j_report, "scan_action", json_object_new_string(a6o_action_pretty_str(report->action)));

	if (report->mod_name != NULL)
		json_object_object_add(j_report, "mod_name", json_object_new_string(report->mod_name));

	if (report->mod_report != NULL)
		json_object_object_add(j_report, "mod_report", json_object_new_string(report->mod_report));

	return j_report;
}

struct scan_data {
	struct api_client *client;
	time_t last_send_time;
	int last_send_progress;
	struct a6o_on_demand *on_demand;
};

#ifdef linux
static time_t get_milliseconds(void)
{
	struct timeval now;

	if (gettimeofday(&now, NULL) < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "error getting time IHM (%s)", strerror(errno));
		return 0;
	}

	return now.tv_sec * 1000 + now.tv_usec / 1000;
}
#endif

#ifdef _WIN32
static time_t get_milliseconds( ) {

	time_t ms = 0;
	struct _timeb tb;

	_ftime64_s(&tb);
	ms = tb.time * 1000 + tb.millitm;

	return ms;
}
#endif

#define SEND_PERIOD 200  /* milliseconds */

static void scan_callback(struct a6o_report *report, void *callback_data)
{
	struct json_object *j_report;
	struct scan_data *scan_data = (struct scan_data *)callback_data;
	time_t now = get_milliseconds();

	if (report->status == ARMADITO_CLEAN
		&& report->progress != 100
		&& scan_data->last_send_progress != REPORT_PROGRESS_UNKNOWN
		&& scan_data->last_send_progress == report->progress
		&& scan_data->last_send_time != 0
		&& (now - scan_data->last_send_time) < SEND_PERIOD)
		return;

	j_report = report_json(report);

	g_async_queue_push(scan_data->client->event_queue, j_report);

	scan_data->last_send_time = now;
	scan_data->last_send_progress = report->progress;
}

static int scan_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out)
{
	return 0;
}

static int poll_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out)
{
	int64_t token;
	struct api_client *client;

	api_get_token(connection, &token);
	client = api_handler_get_client(a, token);

	if (client != NULL) {
		*out = (struct json_object *)g_async_queue_pop(client->event_queue);
		return 0;
	}

	return 1;
}

struct api_data {
	api_cb_t api_cb;
	int64_t token;
	int http_status_code;
	struct MHD_Response *error_response;
};

static int api_handler_pre_check(struct api_handler *a, struct MHD_Connection *connection,
	const char *path, enum http_method method, struct api_data *data)
{
	/* return a HTTP 404 if path is not valid */
	data->api_cb = get_api_cb(path);
	if (data->api_cb == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "request to API invalid path %s", path);

		data->http_status_code = MHD_HTTP_NOT_FOUND;
		data->error_response = a->response_404;

		return 0;
	}

	/* return a HTTP 403 (forbidden) if no User-Agent header */
	if (api_get_user_agent(connection) == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "request to API path %s has no User-Agent header", path);

		data->http_status_code = MHD_HTTP_FORBIDDEN;
		data->error_response = a->response_403;

		return 0;
	}

	/* if endpoint is not /token and if no token in HTTP headers, return a HTTP 400 (bad request) */
	if (strcmp(path, "/token") && api_get_token(connection, &data->token) == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "request to API path %s has no " API_TOKEN_HEADER " header", path);
		data->http_status_code = MHD_HTTP_FORBIDDEN;
		data->error_response = a->response_400;

		return 0;
	}

	/* must verify Content-Type and encoding and return HTTP 415 Unsupported Media Type if invalid */

	/* return a HTTP 400 (bad request) if request parameters are not valid */
	/* TODO */

	return 1;
}

int api_handler_serve(struct api_handler *a, struct MHD_Connection *connection,
	const char *path, enum http_method method, const char *post_data)
{
	struct api_data data;
	const char *json_buff;
	struct json_object *j_response;
	struct MHD_Response *response;
	int ret;

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "request to API: path %s", path);
	connection_debug(connection);

	if (!api_handler_pre_check(a, connection, path, method, &data))
		return MHD_queue_response(connection, data.http_status_code, data.error_response);

	j_response = NULL;
	ret = (*data.api_cb)(a, connection, NULL, &j_response);
	json_buff = json_object_to_json_string(j_response);

	response = MHD_create_response_from_buffer(strlen(json_buff), (char *)json_buff, MHD_RESPMEM_MUST_COPY);
	if (response == NULL) {
		json_object_put(j_response); /* free the json object */
		return MHD_NO;
	}

	json_object_put(j_response); /* free the json object */

	MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
	MHD_add_response_header(response, MHD_HTTP_HEADER_CONNECTION, "close");
	MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
	/* Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept */

	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

static struct MHD_Response *create_std_response(const char *json)
{
	struct MHD_Response *resp;

	resp = MHD_create_response_from_buffer(strlen(json), (char *)json, MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header(resp,  MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
	MHD_add_response_header(resp, MHD_HTTP_HEADER_CONNECTION, "close");

	return resp;
}

struct api_handler *api_handler_new(void)
{
	struct api_handler *a = malloc(sizeof(struct api_handler));

	a->client_table = g_hash_table_new_full(g_int64_hash, g_int64_equal, (GDestroyNotify)free, (GDestroyNotify)api_client_destroy);

	a->response_400 = create_std_response(JSON_400);
	a->response_403 = create_std_response(JSON_403);
 	a->response_404 = create_std_response(JSON_404);
	a->response_415 = create_std_response(JSON_415);

	return a;
}

static int api_handler_add_client(struct api_handler *a, int64_t token)
{
	int64_t *p_key;
	struct api_client *client;

	p_key = malloc(sizeof(int64_t));
	*p_key = token;

	if (g_hash_table_contains(a->client_table, p_key)) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "API token %lld already registered", token);
		free(p_key);
		return 0;
	}

	client = api_client_new();

	return g_hash_table_insert(a->client_table, p_key, client);
}

static struct api_client *api_handler_get_client(struct api_handler *a, int64_t token)
{
	int64_t *p_key;
	struct api_client *c;

	p_key = malloc(sizeof(int64_t));
	*p_key = token;

	c = g_hash_table_lookup(a->client_table, p_key);

	if (c == NULL)
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "API token %lld is not registered", token);

	free(p_key);

	return c;
}

static int api_handler_remove_client(struct api_handler *a, int64_t token)
{
	int64_t *p_key;

	p_key = malloc(sizeof(int64_t));
	*p_key = token;

	if (!g_hash_table_contains(a->client_table, p_key)) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "API token %lld is not registered", token);
		free(p_key);
		return 0;
	}

	return g_hash_table_remove(a->client_table, p_key);
}
