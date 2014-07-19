#include <stdio.h>

const char *module3_files[] = {
  "ELF 64-bit LSB executable",
  "ELF 64-bit LSB shared object",
  "ELF 64-bit LSB relocatable",
  NULL,
};

enum umw_scan_status module3_scan(const char *path, void *mod_data)
{
  return 0;
}

void *module3_init(void)
{
}

void module3_install(void)
{
  fprintf(stderr, "module3 installed ok\n");
}
