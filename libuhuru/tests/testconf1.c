#include <libuhuru/core.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  struct uhuru_conf *conf;
  uhuru_error *error = NULL;
  const char **sections, **p;
  size_t len;
  
  assert(argc > 2);

  conf = uhuru_conf_new();

  if (uhuru_conf_load_file(conf, argv[1], &error))
    return 1;

  if (uhuru_conf_save_file(conf, argv[2], &error))
    return 2;

  sections = uhuru_conf_get_sections(conf, &len);

  printf("== sections\n");

  for(p = sections; *p != NULL; p++)
    printf("section %s\n", *p);

  for(p = sections; *p != NULL; p++)
    free((void *)*p);

  free((void *)sections);

  uhuru_conf_free(conf);

  return 0;
}
