#include <libarmadito-ipc/armadito-ipc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

static int test_send_base_info(void)
{
	struct a6o_base_info *binf = base_info_new("daily.cld", 1, "0.1", 0xdeadbeef, "/var/lib/clamav/daily.cld");
	int ret;
	char *buffer;
	size_t buff_size;

	a6o_ipc_serialize(a6o_base_info, binf, &buffer, &buff_size);

	fwrite(buffer, buff_size, 1, stdout);
	fprintf(stdout, "\r\n\r\n");

	return ret;
}

#define N_BASES 3

static int test_send_module_info(void)
{
	struct a6o_module_info *mod = malloc(sizeof(struct a6o_module_info));
	int ret;
	char *buffer;
	size_t buff_size;

	mod->name = strdup("moduleH1");
	mod->mod_status = A6O_UPDATE_NON_AVAILABLE;
	mod->mod_update_ts = 69;
	mod->base_infos = calloc(N_BASES + 1, sizeof(void *));
	mod->base_infos[0] = base_info_new("daily", 1, "0.1", 0xdeadbeef, "/var/lib/clamav/daily.cld");
	mod->base_infos[1] = base_info_new("main", 2, "0.2", 0xcacaface, "/var/lib/clamav/main.cvd");
	mod->base_infos[2] = base_info_new("bytecode", 3, "0.3", 0xfeedface, "/var/lib/clamav/bytecode.cld");
	mod->base_infos[3] = NULL;

	a6o_ipc_serialize(a6o_module_info, mod, &buffer, &buff_size);

	fwrite(buffer, buff_size, 1, stdout);
	fprintf(stdout, "\r\n\r\n");

	return ret;
}

int main(int argc, char **argv)
{
	if (argc < 2)
		exit(EXIT_FAILURE);

	if (!strcmp(argv[1], "base"))
		return test_send_base_info();
	else if (!strcmp(argv[1], "module"))
		return test_send_module_info();

	return 1;
}
