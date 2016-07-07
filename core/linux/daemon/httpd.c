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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libarmadito.h>

#include "httpd.h"

#define PAGE_404 "<html><head><title>not found!</title></head><body>not found!</body></html>\n"

struct httpd {
	int listen_sock;
	struct MHD_Daemon *daemon;
	struct MHD_Response *response_404;
	magic_t magic;
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

      lseek (fd, 0, SEEK_SET);

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

	if (!is_url_valid(url))
		return MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, h->response_404);

	path = get_path_from_url(url);
	fd = do_open(path, &file_size);

	if (fd < 0)
		return MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, h->response_404);

	mime_type = get_mime_type(h->magic, fd, path);
	free((void *)path);

	if (mime_type == NULL) {
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

static struct json_object *json_response_get(const char *url)
{
	struct json_object *obj ;
	struct json_object *params;

	obj = json_object_new_object();
	json_object_object_add(obj, "response", json_object_new_string("pong"));
	params = json_object_new_object();
	json_object_object_add(params, "value", json_object_new_int(rand()));
	json_object_object_add(obj, "params", json_object_get(params));

	return obj;
}

static int api_serve(struct MHD_Connection *connection, const char *url)
{
	struct MHD_Response *response;
	struct json_object *j_response ;
	const char *json_buff;
	int ret;

	j_response = json_response_get(url);
	json_buff = json_object_to_json_string(j_response);

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
	struct httpd *h = (struct httpd *)con_cls;

	fprintf(stderr, "got request for: %s\n", url);

	if (strcmp(method, MHD_HTTP_METHOD_GET) != 0
		&& strcmp(method, MHD_HTTP_METHOD_POST) != 0)
		return MHD_NO; /* should return a response saying "invalid method" */

	url_path = get_path(url);

	if (!strncmp(url_path, "/api", 4))
		return api_serve(connection, url_path);

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
	socklen_t addrlen;

	client_sock = accept(h->listen_sock, &client_addr, &addrlen);

	if (client_sock < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "accept() failed (%s)", strerror(errno));
		return FALSE;
	}

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, "accepted client connection: fd = %d", client_sock);

	r = MHD_add_connection (h->daemon, client_sock, (struct sockaddr *)&client_addr, addrlen);

	if (r != MHD_YES) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, "accepted client connection: fd = %d", client_sock);
		return FALSE;
	}

	return TRUE;
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

	h->response_404 = MHD_create_response_from_buffer(strlen(PAGE_404), (char *)PAGE_404, MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header(h->response_404,  MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");

	h->magic = magic_open(MAGIC_MIME_TYPE);
	magic_load(h->magic, NULL);

	channel = g_io_channel_unix_new(h->listen_sock);
	g_io_add_watch(channel, G_IO_IN, httpd_listen_cb, h);

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_INFO , "HTTP server started on port %d\n", port);

	return h;
}

int httpd_destroy(struct httpd *h)
{
	MHD_stop_daemon(h->daemon);
	magic_close(h->magic);
}


