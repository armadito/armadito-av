#include "armadito-config.h"

#include <libarmadito/armadito.h>
#include <libjrpc/jrpc.h>

#include "test.h"
#include "unix.h"

#include <fcntl.h>
#include <jansson.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

#define ERR_DIVIDE_BY_ZERO  ((unsigned char)1)
#define ERR_SQRT_OF_NEGATIVE  ((unsigned char)2)
#define ERR_INVALID_ACTION  ((unsigned char)3)

static int op_add(int *p_res, int op1, int op2)
{
	*p_res = op1 + op2;

	return JRPC_OK;
}

static int op_div(int *p_res, int op1, int op2)
{
	if (op2 == 0)
		return ERR_DIVIDE_BY_ZERO;

	*p_res = op1 / op2;

	return JRPC_OK;
}

static int op_sqrt(int *p_res, int op1, int op2)
{
	if (op1 < 0)
		return ERR_SQRT_OF_NEGATIVE;

	*p_res = (int)sqrt(op1);

	return JRPC_OK;
}

static int operator_method(json_t *params, json_t **result, int (*operator)(int *, int, int))
{
	struct operands *s_op;
	struct operands *s_res = operands_new(1);
	int ret;

	if ((ret = JRPC_JSON2STRUCT(operands, params, &s_op)))
		return ret;

	switch(s_op->opt) {
	case OP_INT:
		s_res->opt = OP_INT;
		ret = (*operator)(&s_res->i_result, s_op->i_op1, s_op->i_op2);
		break;
	}

	if (ret)
		return ret;

	if ((ret = JRPC_STRUCT2JSON(operands, s_res, result)))
		return ret;

	return JRPC_OK;
}

static int add_method(struct jrpc_connection *conn, json_t *params, json_t **result)
{
	return operator_method(params, result, op_add);
}

static int div_method(struct jrpc_connection *conn, json_t *params, json_t **result)
{
	return operator_method(params, result, op_div);
}

static int sqrt_method(struct jrpc_connection *conn, json_t *params, json_t **result)
{
	return operator_method(params, result, op_sqrt);
}

static struct jrpc_connection *client_connection;

static pthread_t notify_thread_1;
static pthread_t notify_thread_2;

static void *notify_thread_fun(void *arg)
{
	unsigned long s = (unsigned long)arg;

	while(1) {
		if (client_connection == NULL)
			break;
		jrpc_notify(client_connection, "do_notify", NULL);
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

static int notify_method(struct jrpc_connection *conn, json_t *params, json_t **result)
{
	struct notify_action *action;
	int ret;

	if ((ret = JRPC_JSON2STRUCT(notify_action, params, &action)))
		return ret;

	if (!strcmp(action->whot, "start"))
		notify_start();
	else if (!strcmp(action->whot, "stop"))
		notify_stop();
	else
		return ERR_INVALID_ACTION;

	return JRPC_OK;
}


int main(int argc, char **argv)
{
	struct jrpc_mapper *server_mapper;
	int listen_sock;

	listen_sock = unix_server_listen(SOCKET_PATH);

	if (listen_sock < 0) {
		perror("cannot listen on " SOCKET_PATH);
		exit(EXIT_FAILURE);
	}

	server_mapper = jrpc_mapper_new();
	jrpc_mapper_add(server_mapper, "add", add_method);
	jrpc_mapper_add(server_mapper, "div", div_method);
	jrpc_mapper_add(server_mapper, "sqrt", sqrt_method);
	jrpc_mapper_add(server_mapper, "notify", notify_method);

	jrpc_mapper_add_error_message(server_mapper, ERR_DIVIDE_BY_ZERO, "divide by zero");
	jrpc_mapper_add_error_message(server_mapper, ERR_SQRT_OF_NEGATIVE, "square root of negative number");
	jrpc_mapper_add_error_message(server_mapper, ERR_INVALID_ACTION, "invalid notification action");

	while (1) {
		int client_sock, ret;
		int *p_client_sock;
		struct jrpc_connection *conn;
		struct sockaddr_un addr;
		socklen_t addr_len = sizeof(addr);

		memset(&addr, 0, addr_len);
		if ((client_sock = accept(listen_sock, (struct sockaddr *)&addr, &addr_len)) < 0)
			exit(EXIT_FAILURE);

		fprintf(stderr, "got connection from %s\n", addr.sun_path);

		p_client_sock = malloc(sizeof(int));
		*p_client_sock = client_sock;

		conn = jrpc_connection_new(server_mapper, NULL);

		client_connection = conn;

		jrpc_connection_set_read_cb(conn, unix_fd_read_cb, p_client_sock);
		jrpc_connection_set_write_cb(conn, unix_fd_write_cb, p_client_sock);

		while ((ret = jrpc_process(conn)) != JRPC_EOF)
			;

		client_connection = NULL;
		jrpc_connection_free(conn);

		fprintf(stderr, "disconnected %s\n", addr.sun_path);
	}

	return 0;
}
