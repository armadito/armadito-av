#include <libumwsu/module.h>
#include <stdio.h>

const char *testmodule_mime_types[] = {
  "application/x-sharedlib",
  "application/x-object",
  "application/x-executable",
  NULL,
};

enum umw_status testmodule_scan(const char *path, void *mod_data)
{
  return UMW_CLEAN;
}

enum umw_mod_status testmodule_init(void **pmod_data)
{
  *pmod_data = NULL;

  return UMW_MOD_OK;
}

void testmodule_install(void)
{
  fprintf(stderr, "testmodule installed ok\n");
}
