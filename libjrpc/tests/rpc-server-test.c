#include <libarmadito/armadito.h>
#include <libjrpc/jrpc.h>

#include "test.h"
#include "unix.h"

#include <fcntl.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int op_add(int *p_res, int op1, int op2)
{
	*p_res = op1 + op2;

	return JRPC_OK;
}

static int op_div(int *p_res, int op1, int op2)
{
	if (op2 == 0)
		return JRPC_ERR_METHOD_CALL_ERROR;

	*p_res = op1 / op2;

	return JRPC_OK;
}

static int operator_method(json_t *params, json_t **result, int (*operator)(int *, int, int))
{
	struct operands *s_op;
	struct operands *s_res = operands_new();
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

static int add_method(json_t *params, json_t **result, void *connection_data)
{
	return operator_method(params, result, op_add);
}

static int div_method(json_t *params, json_t **result, void *connection_data)
{
	return operator_method(params, result, op_div);
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

		jrpc_connection_set_read_cb(conn, unix_fd_read_cb, p_client_sock);
		jrpc_connection_set_write_cb(conn, unix_fd_write_cb, p_client_sock);

		while((ret = jrpc_process(conn)) == JRPC_OK)
			;

		if (ret != JRPC_EOF)
			return ret;
	}

	return 0;
}
