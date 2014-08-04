#include <libumwsu/module.h>
#include <stdio.h>

enum umw_mod_status module2_init(void **pmod_data)
{
  fprintf(stderr, "module2 installed ok\n");
}
