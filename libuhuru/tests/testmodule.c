#include <libumwsu/module.h>
#include <stdio.h>

const char *testmodule_mime_types[] = {
  "application/x-sharedlib",
  "application/x-object",
  "application/x-executable",
  NULL,
};

enum umwsu_status testmodule_scan(const char *path, void *mod_data)
{
  return UMWSU_CLEAN;
}

enum umwsu_mod_status testmodule_init(void **pmod_data)
{
  *pmod_data = NULL;

  return UMWSU_MOD_OK;
}

void testmodule_install(void)
{
  fprintf(stderr, "testmodule installed ok\n");
}
