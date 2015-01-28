#include <libumwsu/scan.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

struct umwsu_watch_options {
  int recursive;
  /* more options later */
  char *file_or_dir;
};

static struct option long_options[] = {
  {"help",         no_argument,       0, 'h'},
  {"recursive",    no_argument,       0, 'r'},
  {0, 0, 0, 0}
};

static void usage(void)
{
  fprintf(stderr, "usage: uhuru-watch [options] FILE|DIR\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Uhuru antivirus scanner directory watch\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  --help  -h               print help and quit\n");
  fprintf(stderr, "\n");

  exit(1);
}

static void parse_options(int argc, char *argv[], struct umwsu_watch_options *opts)
{
  int c;

  opts->recursive = 0;
  opts->file_or_dir = NULL;

  while (1) {
    int option_index = 0;

    c = getopt_long(argc, argv, "hrtn", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 'h':
      usage();
      break;
    case 'r':
      opts->recursive = 1;
      break;
    default:
      usage();
    }
  }

  if (optind < argc)
    opts->file_or_dir = argv[optind];

  if (opts->file_or_dir == NULL)
    usage();
}

static void report_print_callback(struct umwsu_report *report, void *callback_data)
{
  FILE *out = (FILE *)callback_data;

  umwsu_report_print(report, out);
}

int main(int argc, char **argv)
{
  struct umwsu *u;
  struct umwsu_watch_options opts;
  struct umwsu_watch_event watch_event;
  
  parse_options(argc, argv, &opts);

  u = umwsu_open();

  umwsu_set_verbose(u, 1);

  umwsu_watch(u, opts.file_or_dir);

  watch_event.full_path = NULL;

  while (!umwsu_watch_next_event(u, &watch_event)) {
    struct umwsu_scan *scan;
    enum umwsu_scan_flags flags = UMWSU_SCAN_RECURSE | UMWSU_SCAN_THREADED;

    switch(watch_event.event_type) {
    case UMWSU_WATCH_DIRECTORY_CREATE:
      scan = umwsu_scan_new(u, watch_event.full_path, flags);
      umwsu_scan_add_callback(scan, report_print_callback, stdout);
      umwsu_scan_start(scan);
      umwsu_scan_finish(scan);
      umwsu_scan_free(scan);
      break;
    default:
      break;
    }
  }

  umwsu_close(u);

  return 0;
}
