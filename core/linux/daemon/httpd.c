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

#define _GNU_SOURCE

#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <json.h>
#include <magic.h>
#include <microhttpd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <libarmadito.h>

#include "httpd.h"
#include "reportp.h"

#define PAGE_404 "<html><head><title>not found!</title></head><body>not found!</body></html>\n"
#define PAGE_403 "<html><head><title>forbidden!</title></head><body>Request forbidden. Make sure your request has a User-Agent header.</body></html>\n"

struct httpd {
	int listen_sock;
	struct MHD_Daemon *daemon;
	struct MHD_Response *response_404;
	struct MHD_Response *response_403;
	magic_t magic;
	GAsyncQueue *event_queue;
};

#define MAGIC_HEADER_SIZE (8 * 1024)

static int is_url_valid(const char *url)
{
	if (!strcmp(url, "/"))
		return 0;

	if (strstr(url, "..") != NULL)
		return 0;

	return 1;
}

static const char *get_path_from_url(const char *url)
{
	char *path;

	asprintf(&path, A6O_MHD_CONTENT_ROOT_DIR "%s", url);

	return path;
}

static int do_open(const char *path, off_t *p_file_size)
{
	int fd;
	struct stat buf;

	fd = open(path, O_RDONLY);

	if (fd < 0)
		return -1;

	if (fstat (fd, &buf) < 0) {
		close(fd);
		return -1;
	}

	if (!S_ISREG (buf.st_mode)) {
		close(fd);
		return -1;
	}

	*p_file_size = buf.st_size;

	return fd;
}

static const char *get_mime_type(magic_t magic, int fd, const char *path)
{
      char header[MAGIC_HEADER_SIZE];
      ssize_t nread;
      const char *mime_type = NULL;
      const char *dot;

      dot = strrchr(path, '.');
      if (dot != NULL && dot != path && !strcmp(dot + 1, "css"))
	      return "text/css";

      nread = read(fd, header, MAGIC_HEADER_SIZE);
      if (nread != -1)
	mime_type = magic_buffer(magic, header, nread);

      lseek(fd, 0, SEEK_SET);

      return mime_type;
}

static int content_serve(struct httpd *h, struct MHD_Connection *connection, const char *url)
{
	int fd;
	const char *path;
	off_t file_size = 0;
	const char *mime_type;
	struct MHD_Response *response;
	int ret;

	if (!is_url_valid(url)) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "url %s is not valid", url);
		return MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, h->response_404);
	}

	path = get_path_from_url(url);
	fd = do_open(path, &file_size);

	if (fd < 0) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "cannot open path %s", path);
		return MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, h->response_404);
	}

	mime_type = get_mime_type(h->magic, fd, path);
	free((void *)path);

	if (mime_type == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "cannot get mime type of path %s", path);
		close(fd);
		return MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, h->response_404);
	}

	response = MHD_create_response_from_fd(file_size, fd);
	if (response == NULL) {
		close (fd);
		return MHD_NO;
	}

	 MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, mime_type);
	 MHD_add_response_header(response, MHD_HTTP_HEADER_CACHE_CONTROL, "no-cache, no-store, must-revalidate");
	 MHD_add_response_header(response, MHD_HTTP_HEADER_PRAGMA, "no-cache");
	 MHD_add_response_header(response, MHD_HTTP_HEADER_EXPIRES, "0");

	 ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	 MHD_destroy_response(response);

	 return ret;
}

/*
   OnDemandProgressEvent:
    allOf:
      - $ref: '#/definitions/Event'
      - type: object
        properties:
          progress:
            type: integer
            format: int32
            description: the progress bar
          malware_count:
            type: integer
            format: int32
            description: malware counter
          suspicious_count:
            type: integer
            format: int32
            description: suspicious counter
          scanned_count:
            type: integer
            format: int32
            description: scanned file counter
* */

static struct a6o_report *report_fake_new(void)
{
	static int count = 0;
	struct a6o_report *r = malloc(sizeof(struct a6o_report));

	a6o_report_init(r, 42, "/foo/bar", count);
	count++;
	count %= 101;

	return r;
}

static struct json_object *json_on_demand_progress_event_new(struct a6o_report *report)
{
	struct json_object *j_report;;

	j_report = json_object_new_object();

	json_object_object_add(j_report, "eventType", json_object_new_string("OnDemandProgressEvent"));
 	json_object_object_add(j_report, "progress", json_object_new_int(report->progress));
	json_object_object_add(j_report, "malware_count", json_object_new_int(report->malware_count));
	json_object_object_add(j_report, "suspicious_count", json_object_new_int(report->suspicious_count));
	json_object_object_add(j_report, "scanned_count", json_object_new_int(report->scanned_count));

	return j_report;
}

static const char *api_get_user_agent(struct MHD_Connection *connection)
{
	return MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_USER_AGENT);
}

void djb2_init(int64_t *hash)
{
  *hash = 5381;	
}

void djb2_hash_buff(const char *buff, size_t len, int64_t *hash)
{
	for ( ; len--; buff++)
		*hash = ((*hash << 5) + *hash) + *buff; /* hash * 33 + c */
}

void djb2_hash_str(const char *str, int64_t *hash)
{
	for ( ; *str; str++)
		*hash = ((*hash << 5) + *hash) + *str; /* hash * 33 + c */
}

static struct json_object *api_gen_token(const char *user_agent)
{
	int64_t token;
	char c;
	time_t now;
	struct json_object *j_token;

	djb2_init(&token);
	time(&now);
	djb2_hash_buff((const char *)&now, sizeof(time_t), &token);
	djb2_hash_str(user_agent, &token);
	djb2_hash_buff(&c, sizeof(char *), &token);

	if (token < 0)
		token = -token;
		
	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "token %lld", token);

	j_token = json_object_new_object();
	json_object_object_add(j_token, "token", json_object_new_int64(token));

	return j_token;
}

static int api_serve(struct httpd *h, struct MHD_Connection *connection, const char *path)
{
	const char *user_agent, *json_buff;
	int64_t api_token;
	struct json_object *j_response ;
	struct MHD_Response *response;
	int ret;

	/* return a HTTP 404 if endpoint is not valid */
	/* for now, only one endpoint */
	if (strcmp(path, "/token")) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "request to API invalid path %s", path);
		return MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, h->response_404);
	}

	/* return a HTTP 403 (forbidden) if no User-Agent header */
	user_agent = api_get_user_agent(connection);
	if (user_agent == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "request to API path %s has no User-Agent", path);
		return MHD_queue_response(connection, MHD_HTTP_FORBIDDEN, h->response_403);
	}

	/* if endpoint is not /token, return a HTTP 400 (bad request) if no token in HTTP headers */
	/*  TODO */

	/* return a HTTP 400 (bad request) if request parameters are not valid */
	j_response = api_gen_token(user_agent);
	
	json_buff = json_object_to_json_string(j_response);
	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "json buffer %s", json_buff);

	response = MHD_create_response_from_buffer(strlen(json_buff), (char *)json_buff, MHD_RESPMEM_MUST_COPY);
	if (response == NULL) {
		json_object_put(j_response); /* free the json object */
		return MHD_NO;
	}

	json_object_put(j_response); /* free the json object */

	MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
	MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
	/* Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept */

	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

static int old_api_serve(struct httpd *h, struct MHD_Connection *connection, const char *url)
{
	struct MHD_Response *response;
	struct json_object *j_response ;
	const char *json_buff;
	int ret;

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "in api_serve(): event queue %p", h->event_queue);
	j_response = (struct json_object *)g_async_queue_pop(h->event_queue);
	json_buff = json_object_to_json_string(j_response);
	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "json buffer %s", json_buff);

	response = MHD_create_response_from_buffer(strlen(json_buff), (char *)json_buff, MHD_RESPMEM_MUST_COPY);

	json_object_put(j_response); /* free the json object */

	if (response == NULL) {
		return MHD_NO;
	}

	MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
	MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
	/* Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept */

	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

static const char *get_path(const char *url)
{
	if (!strncmp(url, "http://", 7))
		return strstr(url + 7, "/");

	return url;
}

static int answer_to_connection(void *cls, struct MHD_Connection *connection,
				const char *url, const char *method,
				const char *version, const char *upload_data,
				size_t *upload_data_size, void **con_cls)
{
	const char *url_path;
	struct httpd *h = (struct httpd *)cls;

	fprintf(stderr, "got request for: %s\n", url);

	if (strcmp(method, MHD_HTTP_METHOD_GET) != 0
		&& strcmp(method, MHD_HTTP_METHOD_POST) != 0)
		return MHD_NO; /* should return a response saying "invalid method" */

	url_path = get_path(url);

	if (!strncmp(url_path, "/api", 4))
		return api_serve(h, connection, url_path + 4);

	return content_serve(h, connection, url_path);
}

static int open_listen_socket(short port)
{
	int sock, optval, r;
	struct sockaddr_in listening_addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return -1;

	optval = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0)
		return -1;

	listening_addr.sin_family = AF_INET;
	listening_addr.sin_port = htons(port);
	listening_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	r = bind(sock, (struct sockaddr *)&listening_addr, sizeof(listening_addr));
	if (r < 0)
		return -1;

	r = listen(sock, 5);
	if (r < 0)
		return -1;

	return sock;
}

static gboolean httpd_listen_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
	struct httpd *h = (struct httpd *)data;
	int client_sock, r;
	struct sockaddr_in client_addr;
	socklen_t addrlen = 0;

	client_sock = accept(h->listen_sock, &client_addr, &addrlen);

	if (client_sock < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "accept() failed (%s)", strerror(errno));
		return FALSE;
	}

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, "accepted client connection: fd = %d", client_sock);

	r = MHD_add_connection(h->daemon, client_sock, (struct sockaddr *)&client_addr, addrlen);

	if (r != MHD_YES) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, "accepted client connection: fd = %d", client_sock);
		return FALSE;
	}

	return TRUE;
}

static struct MHD_Response *create_std_response(const char *page)
{
	struct MHD_Response *resp;

	resp = MHD_create_response_from_buffer(strlen(page), (char *)page, MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header(resp,  MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");

	return resp;	
}

struct httpd *httpd_new(unsigned short port)
{
	struct httpd *h = malloc(sizeof(struct httpd));
	GIOChannel *channel;

	h->listen_sock = open_listen_socket(port);
	if(h->listen_sock < 0) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "error opening server TCP socket (%s)", strerror(errno));
		free(h);
		return NULL;
	}

	h->daemon = MHD_start_daemon(MHD_USE_NO_LISTEN_SOCKET | MHD_USE_THREAD_PER_CONNECTION, 0, NULL, NULL, answer_to_connection, h, MHD_OPTION_END);

	if (h->daemon == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "error creating microhttpd server");
		close(h->listen_sock);
		free(h);
		return NULL;
	}

	h->response_404 = create_std_response(PAGE_404);
	h->response_403 = create_std_response(PAGE_403);

	h->magic = magic_open(MAGIC_MIME_TYPE);
	magic_load(h->magic, NULL);

	h->event_queue = g_async_queue_new();

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "in httpd_new(): event queue %p", h->event_queue);

	channel = g_io_channel_unix_new(h->listen_sock);
	g_io_add_watch(channel, G_IO_IN, httpd_listen_cb, h);

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_INFO , "HTTP server started on port %d", port);

	return h;
}

static gpointer gen_events(gpointer data)
{
	struct httpd *h = (struct httpd *)data;

	while (1) {
		struct a6o_report *r = report_fake_new();
		struct json_object *jr = json_on_demand_progress_event_new(r);

		g_async_queue_push(h->event_queue, jr);

		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "gen_events: %s %d", r->path, r->progress);

		g_async_queue_lock(h->event_queue);
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "in thread %d events in queue", g_async_queue_length_unlocked(h->event_queue));
		g_async_queue_unlock(h->event_queue);

		sleep(3);
	}

	return NULL;
}

void httpd_gen_fake_events(struct httpd *h)
{
	GThread *gen_events_thread;

	g_async_queue_ref(h->event_queue);
	gen_events_thread = g_thread_new("gen_events", gen_events, h);
}

int httpd_destroy(struct httpd *h)
{
	MHD_stop_daemon(h->daemon);
	magic_close(h->magic);
}


