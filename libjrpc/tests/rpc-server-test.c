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
	struct operands *s_op;
	struct operands *s_res = operands_new();
	int ret;

	if ((ret = JRPC_JSON2STRUCT(operands, params, &s_op)))
		return ret;

	switch(s_op->opt) {
	case OP_INT:
		s_res->opt = OP_INT;
		s_res->i_result = s_op->i_op1 + s_op->i_op2;
		break;
	}

	if ((ret = JRPC_STRUCT2JSON(operands, s_res, result)))
		return ret;

	return JRPC_OK;
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
