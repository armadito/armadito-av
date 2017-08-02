/*
  compile with:
  gcc -Wall -Wno-unused -g -I../include/ -DDEBUG -o rpc-server-test ../buffer.c ../hash.c ../brpc.c unix.c rpc-server-test.c -lm
*/

#include <brpc.h>

#include "test.h"
#include "unix.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

#define ERR_DIVIDE_BY_ZERO  ((unsigned char)1)
#define ERR_SQRT_OF_NEGATIVE  ((unsigned char)2)
#define ERR_INVALID_ACTION  ((unsigned char)3)

static int add_method(struct brpc_connection *conn, const brpc_buffer_t *params, brpc_buffer_t **result)
{
	int32_t op1, op2, r;

	op1 = brpc_buffer_get_int32(params, 0, NULL);
	op2 = brpc_buffer_get_int32(params, 1, NULL);

	r = op1 + op2;

	*result = brpc_buffer_new("i", r);

	return BRPC_OK;
}

static int div_method(struct brpc_connection *conn, const brpc_buffer_t *params, brpc_buffer_t **result)
{
	int32_t op1, op2, r;

	op1 = brpc_buffer_get_int32(params, 0, NULL);
	op2 = brpc_buffer_get_int32(params, 1, NULL);

	if (op2 == 0)
		return ERR_DIVIDE_BY_ZERO;

	r = op1 / op2;

	*result = brpc_buffer_new("i", r);

	return BRPC_OK;
}

static int sqrt_method(struct brpc_connection *conn, const brpc_buffer_t *params, brpc_buffer_t **result)
{
	int32_t op1, r;

	op1 = brpc_buffer_get_int32(params, 0, NULL);

	if (op1 < 0)
		return ERR_SQRT_OF_NEGATIVE;

	r = (int32_t)sqrt(op1);

	return BRPC_OK;
}

#if 0
static struct brpc_connection *client_connection;

static pthread_t notify_thread_1;
static pthread_t notify_thread_2;

static void *notify_thread_fun(void *arg)
{
	unsigned long s = (unsigned long)arg;

	while(1) {
		if (client_connection == NULL)
			break;
		brpc_notify(client_connection, "do_notify", NULL);
		sleep(s);
	}

	return NULL;
}


static void notify_start(void)
{
	if (pthread_create(&notify_thread_1, NULL, notify_thread_fun, (void *)2))
		perror("pthread_create");
	if (pthread_create(&notify_thread_2, NULL, notify_thread_fun, (void *)3))
		perror("pthread_create");

	fprintf(stderr, "notifications started\n");
}

static void notify_stop(void)
{
	if (pthread_cancel(notify_thread_1))
		perror("pthread_cancel");
	if (pthread_cancel(notify_thread_2))
		perror("pthread_cancel");

	fprintf(stderr, "notifications stopped\n");
}

static int notify_method(struct brpc_connection *conn, brpc_buffer_t *params, brpc_buffer_t **result)
{
	struct notify_action *action;
	int ret;

	if ((ret = BRPC_JSON2STRUCT(notify_action, params, &action)))
		return ret;

	if (!strcmp(action->whot, "start"))
		notify_start();
	else if (!strcmp(action->whot, "stop"))
		notify_stop();
	else
		return ERR_INVALID_ACTION;

	return BRPC_OK;
}
#endif

int main(int argc, char **argv)
{
	struct brpc_mapper *server_mapper;
	int listen_sock;

	listen_sock = unix_server_listen(SOCKET_PATH);

	if (listen_sock < 0) {
		perror("cannot listen on " SOCKET_PATH);
		exit(EXIT_FAILURE);
	}

	server_mapper = brpc_mapper_new();
	brpc_mapper_add(server_mapper, METHOD_ADD, add_method);
	brpc_mapper_add(server_mapper, METHOD_DIV, div_method);
	brpc_mapper_add(server_mapper, METHOD_SQRT, sqrt_method);
	/* brpc_mapper_add(server_mapper, METHOD_NOTIFY, notify_method); */

	/* brpc_mapper_add_error_message(server_mapper, ERR_DIVIDE_BY_ZERO, "divide by zero"); */
	/* brpc_mapper_add_error_message(server_mapper, ERR_SQRT_OF_NEGATIVE, "square root of negative number"); */
	/* brpc_mapper_add_error_message(server_mapper, ERR_INVALID_ACTION, "invalid notification action"); */

	while (1) {
		int client_sock, ret;
		int *p_client_sock;
		struct brpc_connection *conn;
		struct sockaddr_un addr;
		socklen_t addr_len = sizeof(addr);

		memset(&addr, 0, addr_len);
		if ((client_sock = accept(listen_sock, (struct sockaddr *)&addr, &addr_len)) < 0)
			exit(EXIT_FAILURE);

		fprintf(stderr, "got connection from %s\n", addr.sun_path);

		p_client_sock = malloc(sizeof(int));
		*p_client_sock = client_sock;

		conn = brpc_connection_new(server_mapper, NULL);

		brpc_connection_set_read_cb(conn, unix_fd_read_cb, p_client_sock);
		brpc_connection_set_write_cb(conn, unix_fd_write_cb, p_client_sock);

		while ((ret = brpc_connection_process(conn)) != BRPC_EOF)
			;

		brpc_connection_free(conn);

		fprintf(stderr, "disconnected %s\n", addr.sun_path);
	}

	return 0;
}
