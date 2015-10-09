#ifndef _LIBUHURU_GETOPT_H_
#define _LIBUHURU_GETOPT_H_

#include <stdlib.h>

struct opt {
  const char *long_form;
  char short_form;
  int need_arg;
  int is_set;
  const char *value;
};

int get_opt(struct opt *def, int argc, const char **argv);

#endif
