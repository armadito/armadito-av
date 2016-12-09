#include <libarmadito/armadito.h>
#include <libarmadito-rpc/armadito-rpc.h>

#include "test.h"
#include "libtest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <jansson.h>

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
	fprintf(stderr, "The callback has been called, result is %lld\n", json_integer_value(json_object_get(result, "result")));
}

static int test_call(struct jrpc_connection *conn)
{
	json_t *operands = json_object();

	json_object_set(operands, "op1", json_integer(1));
	json_object_set(operands, "op2", json_integer(2));
	/* struct operands o; */
	/* o.op1 = 1; */
	/* o.op2 = 2; */

	return jrpc_call(conn, "add", operands, simple_cb, NULL);
}

int main(int argc, char **argv)
{
	int *pfd = malloc(sizeof(int));
	struct jrpc_connection *conn;

	conn = jrpc_connection_new(NULL, NULL);
	*pfd = STDOUT_FILENO;
	jrpc_connection_set_write_cb(conn, unix_fd_write_cb, pfd);

	test_call(conn);

	return 0;
}
