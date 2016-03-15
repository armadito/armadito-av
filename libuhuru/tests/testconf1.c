#include <libuhuru/core.h>

#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  struct uhuru_conf *conf;
  uhuru_error *error = NULL;
  
  assert(argc > 2);

  conf = uhuru_conf_new();

  if (uhuru_conf_load_file(conf, argv[1], &error))
    return 1;

  if (uhuru_conf_save_file(conf, argv[2], &error))
    return 2;

  uhuru_conf_free(conf);

  return 0;
}
