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

int opt_parse(struct opt *opt, int argc, const char **argv);
int opt_is_set(struct opt *opt, const char *opt_name);
const char *opt_value(struct opt *opt, const char *opt_name, const char *default_value);

#endif
