#include "getopt.h"

#include <string.h>

static void clear_opt(struct opt *opt)
{
  while(opt->long_form != NULL) {
    opt->is_set = 0;
    opt->value = NULL;

    opt++;
  }
}

static struct opt *find_long_opt(struct opt *opt, const char *arg)
{
  char *p = strchr(arg + 2, '=');
  int l = p - arg - 2;

  while(opt->long_form != NULL) {
    int found;

    if (p == NULL)
      found = !strcmp(arg + 2, opt->long_form);
    else
      found = !strncmp(arg + 2, opt->long_form, l);

    if  (found) {
      opt->is_set = 1;

      return opt;
    }

    opt++;
  }

  return NULL;
}

static struct opt *find_short_opt(struct opt *opt, const char *arg)
{
  while(opt->long_form != NULL) {
    if  (arg[1] == opt->short_form && arg[2] == '\0') {
      opt->is_set = 1;

      return opt;
    }

    opt++;
  }

  return NULL;
}

#define is_option(a) ((a)[0] == '-')

#define is_long_option(a) ((a)[1] == '-')

int opt_parse(struct opt *opt, int argc, const char **argv)
{
  int i;
  struct opt *found_opt;
  const char *o, *a;

  clear_opt(opt);

  for (i = 1; i < argc && is_option(argv[i]); i++) {
    o = argv[i];

    if (is_long_option(o)) {
      if ((found_opt = find_long_opt(opt, o)) == NULL)
	return -1;

      if (found_opt->need_arg) {
	if ((a = strchr(o + 2, '=')) == NULL)
	  return -1;

	found_opt->value = a + 1;
      }
    } else {
      if ((found_opt = find_short_opt(opt, o)) == NULL)
	return -1;

      if (found_opt->need_arg) {
	if (++i >= argc)
	  return -1;

	found_opt->value = argv[i];
      }
    }
  }

  return i;
}

int opt_is_set(struct opt *opt, const char *opt_name)
{
  while(opt->long_form != NULL) {
    if (!strcmp(opt_name, opt->long_form))
      return opt->is_set;

    opt++;
  }

  return 0;
}

const char *opt_value(struct opt *opt, const char *opt_name, const char *default_value)
{
  while(opt->long_form != NULL) {
    if (!strcmp(opt_name, opt->long_form) && opt->is_set)
      return opt->value;

    opt++;
  }

  return default_value;
}

