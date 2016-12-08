#include <libarmadito/armadito.h>
#include <libarmadito-rpc/armadito-rpc.h>

#include "test.h"

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
	struct a6o_rpc_mapper *server_mapper;
	struct a6o_rpc_connection *conn;
	char buffer[1024];
	size_t n_read;

	server_mapper = a6o_rpc_mapper_new();
	a6o_rpc_mapper_add(server_mapper, "add", add_method);

	conn = a6o_rpc_connection_new(server_mapper, STDOUT_FILENO, NULL);

	n_read = fread(buffer, 1, sizeof(buffer), stdin);
	if (n_read < 0)
		exit(1);

	return a6o_rpc_process(conn, buffer, n_read);
}
