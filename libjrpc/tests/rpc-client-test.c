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
#include <unistd.h>

static struct a6o_base_info *base_info_new(const char *name, time_t base_update_ts, const char *version, size_t signature_count, const char *full_path)
{
	struct a6o_base_info *b = malloc(sizeof(struct a6o_base_info));

	b->name = strdup(name);
	b->base_update_ts = base_update_ts;
	b->version = version;
	b->signature_count = signature_count;
	b->full_path = strdup(full_path);

	return b;
}

#define N_BASES 3

static struct a6o_module_info *module_info_new(void)
{
	struct a6o_module_info *mod = malloc(sizeof(struct a6o_module_info));

	mod->name = strdup("moduleH1");
	mod->mod_status = A6O_UPDATE_NON_AVAILABLE;
	mod->mod_update_ts = 69;
	mod->base_infos = calloc(N_BASES + 1, sizeof(void *));
	mod->base_infos[0] = base_info_new("daily", 1, "0.1", 0xdeadbeef, "/var/lib/clamav/daily.cld");
	mod->base_infos[1] = base_info_new("main", 2, "0.2", 0xabbaface, "/var/lib/clamav/main.cvd");
	mod->base_infos[2] = base_info_new("bytecode", 3, "0.3", 0xfeedface, "/var/lib/clamav/bytecode.cld");
	mod->base_infos[3] = NULL;

	return mod;
}

static int test_notification(struct jrpc_connection *conn)
{
	json_t *params;
	struct a6o_module_info *mod = module_info_new();

	/* JRPC_STRUCT2JSON(a6o_module_info, mod, &params); */

	return jrpc_notify(conn, "status", params);
}

static void simple_cb(json_t *result, void *user_data)
{
	struct operands *op;
	int ret;

	if ((ret = JRPC_JSON2STRUCT(operands, result, &op)))
		return;

	fprintf(stderr, "The callback has been called, result is %d\n", op->i_result);
}

static int test_call(struct jrpc_connection *conn, const char *method, int op1, int op2)
{
	struct operands *s_op = operands_new();
	json_t *j_op;
	int ret;

	s_op->i_op1 = op1;
	s_op->i_op2 = op2;

	if ((ret = JRPC_STRUCT2JSON(operands, s_op, &j_op)))
		return ret;

	return jrpc_call(conn, method, j_op, simple_cb, NULL);
}

static int test_add(struct jrpc_connection *conn, int count)
{
	int op = 0, i, ret = 0;

	for(i = 0; i < count; i++) {
		if ((ret = test_call(conn, "add", op, op + 1)))
			break;
		op++;
	}

	return ret;
}

static int test_notify(struct jrpc_connection *conn, const char *whot)
{
	struct notify_action action;
	json_t *j_action;
	int ret;

	action.whot = whot;

	if ((ret = JRPC_STRUCT2JSON(notify_action, &action, &j_action)))
		return ret;

	return jrpc_notify(conn, "notify", j_action);
}

static int do_notify_method(json_t *params, json_t **result, void *connection_data)
{
	fprintf(stderr, "I have been notified\n");

	return JRPC_OK;
}

static void client_error_handler(struct jrpc_connection *conn, size_t id, int code, const char *message, json_t *data)
{
	if (JRPC_ERR_IS_METHOD_ERROR(code))
		code = JRPC_ERR_CODE_TO_METHOD(code);

	fprintf(stderr, "error handler: id %ld code %d message \"%s\"\n", id, code, message);
}

int main(int argc, char **argv)
{
	struct jrpc_mapper *client_mapper;
	struct jrpc_connection *conn;
	int client_sock;
	int *p_client_sock;
	int ret;

	client_sock = unix_client_connect(SOCKET_PATH, 10);

	if (client_sock < 0) {
		perror("cannot connect to " SOCKET_PATH);
		exit(EXIT_FAILURE);
	}

	client_mapper = jrpc_mapper_new();
	jrpc_mapper_add(client_mapper, "do_notify", do_notify_method);

	conn = jrpc_connection_new(client_mapper, NULL);

	p_client_sock = malloc(sizeof(int));
	*p_client_sock = client_sock;

	jrpc_connection_set_read_cb(conn, unix_fd_read_cb, p_client_sock);
	jrpc_connection_set_write_cb(conn, unix_fd_write_cb, p_client_sock);

	jrpc_connection_set_error_handler(conn, client_error_handler);

	test_add(conn, 10);
	test_call(conn, "div", 9, 3);
	test_call(conn, "div", 9, 0);
	test_call(conn, "sqrt", 4761, 0);
	test_call(conn, "sqrt", -9, 0);
	test_notify(conn, "start");
	test_notify(conn, "foo");

	while((ret = jrpc_process(conn)) != JRPC_EOF)
		;

	return 0;
}
