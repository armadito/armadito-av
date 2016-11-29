#include <libarmadito-ipc/armadito-ipc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void test_receive_base_info(struct a6o_base_info *binf)
{
	printf(" name: %s\n", binf->name);
	printf(" update_ts: %ld\n", binf->base_update_ts);
	printf(" version: %s\n", binf->version);
	printf(" signature_count: 0x%lx\n", binf->signature_count);
	printf(" full_path: %s\n", binf->full_path);
}

static void test_receive_module_info(struct a6o_module_info *mod)
{
	struct a6o_base_info **p;

	printf(" name: %s\n", mod->name);
	printf(" update_ts: %ld\n", mod->mod_update_ts);
	printf(" status: %d\n", mod->mod_status);

	for(p = mod->base_infos; *p != NULL; p++) {
		printf(" base_info[%ld]\n", p - mod->base_infos);
		test_receive_base_info(*p);
	}
}

int main(int argc, char **argv)
{
	int ret;
	char buffer[1024];
	size_t n_read;
	void *p;

	if (argc < 2)
		exit(EXIT_FAILURE);

	n_read = fread(buffer, 1, sizeof(buffer), stdin);

	fprintf(stderr, "deserializing %s\n", buffer);

	ret = a6o_ipc_deserialize(buffer, n_read * sizeof(buffer), &p);

	if (ret != 0) {
		fprintf(stderr, "error deserializing structure\n");
		return ret;
	}

	if (!strcmp(argv[1], "base"))
		test_receive_base_info(p);
	else if (!strcmp(argv[1], "module"))
		test_receive_module_info(p);

	return 0;
}
