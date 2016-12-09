#include <libarmadito/armadito.h>
#include <libarmadito-rpc/armadito-rpc.h>

#include "test.h"
#include "libtest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <jansson.h>

static int add_method(json_t *params, json_t **result, void *connection_data)
{
	int op1, op2;

	op1 = json_integer_value(json_object_get(params, "op1"));
	op2 = json_integer_value(json_object_get(params, "op2"));

	*result = json_object();
	json_object_set(*result, "result", json_integer(op1 + op2));

	return 0;
}


int main(int argc, char **argv)
{
	struct jrpc_mapper *server_mapper;
	int *pfd = malloc(sizeof(int));
	struct jrpc_connection *conn;
	char buffer[1024];
	size_t n_read;

	server_mapper = jrpc_mapper_new();
	jrpc_mapper_add(server_mapper, "add", add_method);

	conn = jrpc_connection_new(server_mapper, NULL);
	*pfd = STDOUT_FILENO;
	jrpc_connection_set_write_cb(conn, unix_fd_write_cb, pfd);

	n_read = fread(buffer, 1, sizeof(buffer), stdin);
	if (n_read < 0)
		exit(1);

	return jrpc_process(conn, buffer, n_read);
}
