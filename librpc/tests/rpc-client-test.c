#include <libarmadito-rpc/armadito-rpc.h>

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

static int test_notification(struct a6o_rpc_connection *conn)
{
	json_t *params;
	struct a6o_module_info *mod = module_info_new();

	A6O_RPC_STRUCT2JSON(a6o_module_info, mod, &params);

	return a6o_rpc_notify(conn, "status", params);
}

static void simple_cb(json_t *result, void *user_data)
{
	fprintf(stderr, "The callback has been called\n");
}

static int test_call(struct a6o_rpc_connection *conn)
{
	json_t *params = json_object();

	json_object_set(params, "path", json_string("/var/tmp"));

	return a6o_rpc_call(conn, "scan", params, simple_cb, NULL);
}

int main(int argc, char **argv)
{
	struct a6o_rpc_connection *conn;

	conn = a6o_rpc_connection_new(STDOUT_FILENO);

	test_notification(conn);

	test_call(conn);

	return 0;
}
