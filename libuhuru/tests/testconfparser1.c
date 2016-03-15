#include "libcore/confparser.h"

#include <assert.h>
#include <stdio.h>

void conf_parser_print_cb(const char *group, const char *key, const char **argv, size_t length, void *user_data)
{
  int i;

  fprintf(stderr, "=> group %s key %s args", group, key);

  for(i = 0; *argv != NULL; i++, argv++)
    fprintf(stderr, " [%d] %s", i, *argv);

  fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
  struct uhuru_conf_parser *cp;
  int ret;

  assert(argc >= 2);

  cp = uhuru_conf_parser_new(argv[1], conf_parser_print_cb, NULL);

  ret = uhuru_conf_parser_parse(cp);

  uhuru_conf_parser_free(cp);

  return ret;
}
