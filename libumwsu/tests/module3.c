#include <libumwsu/module.h>
#include <stdio.h>

const char *module3_files[] = {
  "ELF 64-bit LSB executable",
  "ELF 64-bit LSB shared object",
  "ELF 64-bit LSB relocatable",
  NULL,
};

enum umw_status module3_scan(const char *path, void *mod_data)
{
  return UMW_CLEAN;
}

enum umw_mod_status module3_init(void **pmod_data)
{
  *pmod_data = NULL;

  return UMW_MOD_OK;
}

void module3_install(void)
{
  fprintf(stderr, "module3 installed ok\n");
}
