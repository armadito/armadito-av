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

#include <assert.h>
#include <json.h>
#include <glib.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <libarmadito.h>

#include "api.h"
#include "httpd.h"
#include "debug.h"

#define API_TOKEN_HEADER "X-Armadito-Token"

#define JSON_400 "{\"code\":400, \"message\": \"Bad Request. Make sure your request has a X-Armadito-Token header and if POST request contains valid JSON\"}"
#define JSON_403 "{\"code\":403, \"message\": \"Request forbidden. Make sure your request has a User-Agent header\"}"
#define JSON_404 "{\"code\":404, \"message\": \"Not found\"}"
#define JSON_405 "{\"code\":405, \"message\": \"Method not allowed\"}"
#define JSON_415 "{\"code\":415, \"message\": \"Unsupported Media Type. Content-Type must be application/json\"}"
#define JSON_422 "{\"code\":422, \"message\": \"Unprocessable request. Make sure the JSON request is valid\"}"
#define JSON_500 "{\"code\":500, \"message\": \"Request processing triggered an internal error\"}"

struct api_handler {
	GHashTable *client_table;
	struct MHD_Response *response_400;
	struct MHD_Response *response_403;
	struct MHD_Response *response_404;
	struct MHD_Response *response_405;
	struct MHD_Response *response_415;
	struct MHD_Response *response_422;
	struct MHD_Response *response_500;
	struct json_tokener *tokener;
};

static void api_client_destroy(struct api_client *c);

static struct api_endpoint {
	const char *path;
	api_cb_t api_cb;
	enum http_method accepted_methods;
	int need_token;
} api_endpoint_table[] = {
	{ "/register", register_api_cb, HTTP_METHOD_GET, 0},
	{ "/unregister", unregister_api_cb, HTTP_METHOD_GET, 1},
	{ "/ping", ping_api_cb, HTTP_METHOD_GET, 1},
	{ "/scan", scan_api_cb, HTTP_METHOD_POST, 1},
	{ "/poll", poll_api_cb, HTTP_METHOD_GET, 1},
	{ NULL, NULL},
};

static struct api_endpoint *get_api_endpoint(const char *path)
{
	struct api_endpoint *p;

	for (p = api_endpoint_table; p->path != NULL && strcmp(p->path, path); p++)
		;

	if (p->path != NULL)
		return p;

	return NULL;
}

const char *api_get_user_agent(struct MHD_Connection *connection)
{
	return MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_USER_AGENT);
}

static const char *api_get_content_type(struct MHD_Connection *connection)
{
	const char *s_content_type;
	char *param;

	s_content_type = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_CONTENT_TYPE);
	if (s_content_type == NULL)
		return NULL;

	param = strchr(s_content_type, ';');
	if (param != NULL && param > s_content_type) {
		size_t len = param - s_content_type;
		char *ret = malloc(len + 1);

		assert(len > 0);
		strncpy(ret, s_content_type, len);
		ret[len] = '\0';

		return ret;
	}

	return strdup(s_content_type);
}

const char *api_get_token(struct MHD_Connection *connection, int64_t *p_token)
{
	const char *s_token;

	s_token = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, API_TOKEN_HEADER);
	if (s_token != NULL)
		*p_token = atoll(s_token);

	return s_token;
}

static int api_handler_pre_check(struct api_handler *a, struct MHD_Connection *connection,
	const char *path, enum http_method method, struct api_endpoint **p_endpoint, int64_t *p_token,
	struct MHD_Response **p_error_response)
{
	struct api_endpoint *endpoint;

	/* return a HTTP 404 if path is not valid */
	endpoint = get_api_endpoint(path);
	*p_endpoint = endpoint;
	if (endpoint == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "request to API invalid path %s", path);
		*p_error_response = a->response_404;

		return MHD_HTTP_NOT_FOUND;
	}

	/* return a HTTP 403 forbidden if no User-Agent header */
	if (api_get_user_agent(connection) == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "request to API path %s has no User-Agent header", path);
		*p_error_response = a->response_403;

		return MHD_HTTP_FORBIDDEN;
	}

	/* if endpoint needs token and if no token in HTTP headers, return HTTP 400 bad request */
	if (endpoint->need_token && api_get_token(connection, p_token) == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "request to API path %s has no " API_TOKEN_HEADER " header", path);
		*p_error_response = a->response_400;

		return MHD_HTTP_BAD_REQUEST;
	}

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "accepted methods %d method %d", endpoint->accepted_methods, method);
	/* if method is not in endpoint accepted methods, return HTTP 405 method not allowed */
	if ((endpoint->accepted_methods & method) == 0) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "method not allowed for %s", path);
		*p_error_response = a->response_405;

		return MHD_HTTP_METHOD_NOT_ALLOWED;
	}

	/* if POST, verify Content-Type and encoding and return HTTP 415 Unsupported Media Type if invalid */
	if (method == HTTP_METHOD_POST) {
		const char *content_type = api_get_content_type(connection);

		if (content_type == NULL || strcmp(content_type, "application/json") != 0) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "invalid Content-Type %s", content_type);
			*p_error_response = a->response_415;

			if (content_type != NULL)
				free((void *)content_type);

			return MHD_HTTP_UNSUPPORTED_MEDIA_TYPE;
		}

		free((void *)content_type);
	}

	/* return a HTTP 400 bad request if request parameters are not valid */
	/* TODO */

	return MHD_HTTP_OK;
}

static struct json_object *api_parse_json_request(struct api_handler *a, const char *post_data, size_t post_data_size)
{
	struct json_object *j_request;

	json_tokener_reset(a->tokener);

	j_request = json_tokener_parse_ex(a->tokener, post_data, post_data_size);

	if (j_request == NULL) {
		enum json_tokener_error jerr = json_tokener_get_error(a->tokener);

		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, "error in JSON parsing: %s", json_tokener_error_desc(jerr));
	}

	return j_request;
}

int api_handler_serve(struct api_handler *a, struct MHD_Connection *connection,
	const char *path, enum http_method method, const char *post_data, size_t post_data_size)
{
	struct api_endpoint *endpoint;
	api_cb_t api_cb;
	int64_t token;
	int http_status_code;
	const char *json_buff;
	struct json_object *j_request = NULL, *j_response = NULL;
	struct MHD_Response *response;
	int ret;

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "request to API: path %s", path);

	http_status_code = api_handler_pre_check(a, connection, path, method, &endpoint, &token, &response);
	if (http_status_code != MHD_HTTP_OK)
		return MHD_queue_response(connection, http_status_code, response);

	if (method == HTTP_METHOD_POST && post_data_size) {
		j_request = api_parse_json_request(a, post_data, post_data_size);

		if (j_request == NULL)
			return MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, a->response_400);
	}

	api_cb = endpoint->api_cb;

	ret = (*api_cb)(a, connection, j_request, &j_response);
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
 	a->response_405 = create_std_response(JSON_405);
	a->response_415 = create_std_response(JSON_415);
	a->response_422 = create_std_response(JSON_422);
	a->response_500 = create_std_response(JSON_500);

	a->tokener = json_tokener_new();
	assert(a->tokener != NULL);

	return a;
}

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

int api_handler_add_client(struct api_handler *a, int64_t token)
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

struct api_client *api_handler_get_client(struct api_handler *a, int64_t token)
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

int api_handler_remove_client(struct api_handler *a, int64_t token)
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

int api_client_push_event(struct api_client *client, struct json_object *event)
{
	g_async_queue_push(client->event_queue, event);
}

int api_client_pop_event(struct api_client *client, struct json_object **p_event)
{

}

