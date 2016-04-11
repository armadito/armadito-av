#include <libarmadito.h>

#include "confparser.h"

#include <assert.h>
#include <stdio.h>

int conf_parser_print_cb(const char *section, const char *key, struct a6o_conf_value *value, void *user_data)
{
	int i;

	fprintf(stderr, "=> section %s key %s args", section, key);

#if 0
	for(i = 0; *argv != NULL; i++, argv++)
		fprintf(stderr, " [%d] %s", i, *argv);
#endif

	fprintf(stderr, "\n");

	return 0;
}

int main(int argc, char **argv)
{
	struct a6o_conf_parser *cp;
	int ret;

	assert(argc >= 2);

	cp = a6o_conf_parser_new(argv[1], conf_parser_print_cb, NULL);

	ret = a6o_conf_parser_parse(cp);

	a6o_conf_parser_free(cp);

	return ret;
}
