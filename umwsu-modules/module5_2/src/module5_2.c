#include <libumwsu/module.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

const char *module5_2_mime_types[] = {
  "application/x-sharedlib",
  "application/x-object",
  "application/x-executable",
  NULL,
};

enum umw_status module5_2_scan(const char *path, void *mod_data)
{

  return UMW_CLEAN;
}

enum umw_mod_status module5_2_init(void **pmod_data)
{
  return UMW_MOD_OK;
}

void module5_2_install(void)
{
  fprintf(stderr, "Module5_2 module installed\n");
}
