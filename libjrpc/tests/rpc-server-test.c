#include <libarmadito/armadito.h>
#include <libjrpc/jrpc.h>

#include "test.h"
#include "libtest.h"

#include <fcntl.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int add_method(json_t *params, json_t **result, void *connection_data)
{
	int op1, op2;

	op1 = json_integer_value(json_object_get(params, "op1"));
	op2 = json_integer_value(json_object_get(params, "op2"));

	*result = json_object();
	json_object_set(*result, "result", json_integer(op1 + op2));

	return 0;
}

static void usage(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s INPUT_PIPE OUTPUT_PIPE\n", argv[0]);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	struct jrpc_mapper *server_mapper;
	struct jrpc_connection *conn;
	int *p_input_fd = malloc(sizeof(int));
	int *p_output_fd = malloc(sizeof(int));
	int ret;

	if (argc < 3)
		usage(argc, argv);

	*p_input_fd = open(argv[1], O_RDONLY);
	if (*p_input_fd < 0) {
		perror("cannot open input pipe");
		exit(EXIT_FAILURE);
	}

	*p_output_fd = open(argv[2], O_WRONLY);
	if (*p_output_fd < 0) {
		perror("cannot open output pipe");
		exit(EXIT_FAILURE);
	}

	server_mapper = jrpc_mapper_new();
	jrpc_mapper_add(server_mapper, "add", add_method);

	conn = jrpc_connection_new(server_mapper, NULL);

	jrpc_connection_set_read_cb(conn, unix_fd_read_cb, p_input_fd);
	jrpc_connection_set_write_cb(conn, unix_fd_write_cb, p_output_fd);

	while((ret = jrpc_process(conn)) >= 0) {
		if (ret == 1)
			return 1;
	}

	return ret;
}
