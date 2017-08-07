/*
  compile with:
  - with debug:
  gcc -Wall -Wno-unused -g -I../include/ -DDEBUG -o rpc-client-test ../buffer.c ../hash.c ../brpc.c unix.c rpc-client-test.c
  - without debug:
  gcc -Wall -Wno-unused -g -I../include/ -o rpc-client-test ../buffer.c ../hash.c ../brpc.c unix.c rpc-client-test.c
*/

#include <brpc.h>

#include "test.h"
#include "unix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void simple_cb(const brpc_buffer_t *result, void *user_data)
{
	fprintf(stderr, "The callback has been called, result is %d\n", brpc_buffer_get_int32(result, 0, NULL));
}

static int test_call(struct brpc_connection *conn, uint8_t method, int op1, int op2)
{
	brpc_buffer_t *params = brpc_buffer_new("ii", op1, op2);

	return brpc_call(conn, method, params, simple_cb, NULL);
}

static int test_notify(struct brpc_connection *conn, const char *whot)
{
	brpc_buffer_t *params = brpc_buffer_new("s", whot);

	return brpc_notify(conn, METHOD_NOTIFY_START_STOP, params);
}

static int do_notify_method(struct brpc_connection *conn, const brpc_buffer_t *params, brpc_buffer_t **result)
{
	fprintf(stderr, "I have been notified\n");

	return BRPC_OK;
}

static void client_error_handler(struct brpc_connection *conn, uint32_t id, int code, const char *message)
{
	if (BRPC_ERR_IS_METHOD_ERROR(code))
		code = BRPC_ERR_CODE_TO_METHOD(code);

	fprintf(stderr, "error handler: id %d code %d message \"%s\"\n", id, code, message);
}

int main(int argc, char **argv)
{
	struct brpc_mapper *client_mapper;
	struct brpc_connection *conn;
	int client_sock;
	int *p_client_sock;

	client_sock = unix_client_connect(SOCKET_PATH, 10);

	if (client_sock < 0) {
		perror("cannot connect to " SOCKET_PATH);
		exit(EXIT_FAILURE);
	}

	client_mapper = brpc_mapper_new();
	brpc_mapper_add(client_mapper, METHOD_DO_NOTIFY, do_notify_method);

	conn = brpc_connection_new(client_mapper, NULL);

	p_client_sock = malloc(sizeof(int));
	*p_client_sock = client_sock;

	brpc_connection_set_read_cb(conn, unix_fd_read_cb, p_client_sock);
	brpc_connection_set_write_cb(conn, unix_fd_write_cb, p_client_sock);

	brpc_connection_set_error_handler(conn, client_error_handler);

	test_call(conn, METHOD_ADD, 58, 11);
	test_call(conn, METHOD_DIV, 207, 3);
	test_call(conn, METHOD_DIV, 9, 0);
	test_call(conn, METHOD_SQRT, 4761, 0);
	test_call(conn, METHOD_SQRT, -9, 0);
	test_notify(conn, "start");
	test_notify(conn, "foo");

	while (brpc_connection_process(conn) != BRPC_EOF)
		;

	return 0;
}
