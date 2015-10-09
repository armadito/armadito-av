#include "getopt.h"

#include <stdio.h>

struct opt test_options[] = {
  { .long_form = "help", .short_form = 'h', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "nopt", .short_form = 'n', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "mopt", .short_form = 'm', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "aarg", .short_form = 'a', .need_arg = 1, .is_set = 0, .value = NULL},
  { .long_form = "barg", .short_form = 'b', .need_arg = 1, .is_set = 0, .value = NULL},
  { .long_form = NULL, .short_form = '\0', .need_arg = 0, .is_set = 0, .value = NULL},
};

static void usage(void)
{
  fprintf(stderr, "usage: testopt [options] ARG...\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Test\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  --help -h                print help and quit\n");
  fprintf(stderr, "  --nopt -n                no arg option\n");
  fprintf(stderr, "  --mopt -m                another no arg option\n");
  fprintf(stderr, "  [--aarg=ARG|-a ARG]      arg option\n");
  fprintf(stderr, "  [--barg=ARG|-b ARG]      another arg option\n");
  fprintf(stderr, "\n");

  exit(1);
}

static void print_opt(struct opt *opt)
{
  while(opt->long_form != NULL) {
    printf("long:%s short:%c need_arg:%d is_set:%d value:%s\n", opt->long_form, opt->short_form, opt->need_arg, opt->is_set, opt->value);

    opt++;
  }
}

int main(int argc, const char **argv)
{
  if (get_opt(test_options, argc, argv) < 0)
    usage();

  print_opt(test_options);

  return 0;
}
