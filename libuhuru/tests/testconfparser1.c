#include "lib/conf.h"

#include <assert.h>

int main(int argc, char **argv)
{
  struct uhuru_conf_parser *cp;
  int ret;

  assert(argc >= 2);

  cp = uhuru_conf_parser_new(argv[1]);

  ret = uhuru_conf_parser_parse(cp);

  uhuru_conf_parser_free(cp);

  return ret;
}
