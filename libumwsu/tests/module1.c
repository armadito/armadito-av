#include <libumwsu/module.h>
#include <stdio.h>

enum umw_mod_status module1_init(void **pmod_data)
{
  fprintf(stderr, "module1 installed ok\n");
}
