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
#include <unistd.h>
#include <pthread.h>

static int done = 0;
static int32_t r;

static void simple_cb(json_t *result, void *user_data)
{
	struct operands *op;
	int ret;

	if ((ret = JRPC_JSON2STRUCT(operands, result, &op)))
		return;

	if (op->i_result == 42) {
		r = op->i_result;
		fprintf(stderr, "got the answer: %d\n", r);
		done = 1;
	}
}

static int test_call(struct jrpc_connection *conn, const char *method, int op1, int op2)
{
	struct operands *s_op = operands_new(1);
	json_t *j_op;
	int ret;

	s_op->i_op1 = op1;
	s_op->i_op2 = op2;

	if ((ret = JRPC_STRUCT2JSON(operands, s_op, &j_op)))
		return ret;

	ret = jrpc_call(conn, method, j_op, simple_cb, NULL);

	if (ret)
		fprintf(stderr, "call failed: %d\n", ret);

	return ret;
}

#if 0
static int test_notify(struct brpc_connection *conn, uint8_t method, int op1, int op2)
{
	int ret = brpc_notify(conn, method, "ii", op1, op2);

	if (ret)
		fprintf(stderr, "notify failed: %d\n", ret);

	return BRPC_OK;
}
#endif


static void *cb_thread_fun(void *arg)
{
	struct jrpc_connection *conn = (struct jrpc_connection *)arg;

	while (!done) {
		int ret = jrpc_process(conn);

		if (ret == JRPC_EOF)
			break;
		if (ret != JRPC_OK) {
			fprintf(stderr, "jrpc_connection_process returned %d\n", ret);
			return NULL;
		}
	}

	fprintf(stderr, "thread exiting: done %d\n", done);

	return NULL;
}

static void client_error_handler(struct jrpc_connection *conn, size_t id, int code, const char *message, json_t *data)
{
	if (JRPC_ERR_IS_METHOD_ERROR(code))
		code = JRPC_ERR_CODE_TO_METHOD(code);

	fprintf(stderr, "error handler: id %ld code %d message \"%s\"\n", id, code, message);
}

static void usage(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s COUNT\n", argv[0]);
	exit(EXIT_FAILURE);
}

static size_t get_from_arg(int argc, char **argv)
{
	size_t l;

	if (argc < 2)
		usage(argc, argv);

	if (sscanf(argv[1], "%ld", &l) < 1)
		usage(argc, argv);

	return l;
}

int main(int argc, char **argv)
{
	struct jrpc_connection *conn;
	int client_sock;
	int *p_client_sock;
	pthread_t cb_thread;
	size_t count;
	int ret;

	count = get_from_arg(argc, argv);

	client_sock = unix_client_connect(SOCKET_PATH, 10);

	if (client_sock < 0) {
		perror("cannot connect to " SOCKET_PATH);
		exit(EXIT_FAILURE);
	}
	conn = jrpc_connection_new(NULL, NULL);

	p_client_sock = malloc(sizeof(int));
	*p_client_sock = client_sock;

	jrpc_connection_set_read_cb(conn, unix_fd_read_cb, p_client_sock);
	jrpc_connection_set_write_cb(conn, unix_fd_write_cb, p_client_sock);

	jrpc_connection_set_error_handler(conn, client_error_handler);

	if (pthread_create(&cb_thread, NULL, cb_thread_fun, conn))
		perror("pthread_create");

	while (count--) {
		/* fprintf(stderr, "call %ld\n", count); */
		ret = test_call(conn, "add", 58, 11);
		if (ret)
			break;
	}

	if (!ret)
		ret = test_call(conn, "add", 21, 21);

	fprintf(stderr, "trying to join thread 0x%lx\n", cb_thread);

	/* if (pthread_cancel(cb_thread)) */
	/* 	perror("pthread_cancel"); */

	if (pthread_join(cb_thread, NULL))
		perror("pthread_join");

	jrpc_connection_free(conn);
	free(p_client_sock);

	return ret;
}
