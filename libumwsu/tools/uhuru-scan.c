#include <libumwsu/scan.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

struct umwsu_scan_summary {
  int scanned;
  int malware;
  int suspicious;
  int unhandled;
  int clean;
};

struct umwsu_scan_options {
  int recursive;
  int threaded;
  int no_summary;
  /* more options later */
  char *path;
};

static struct option long_options[] = {
  {"help",         no_argument,       0, 'h'},
  {"recursive",    no_argument,       0, 'r'},
  {"threaded",     no_argument,       0, 't'},
  {"no-summary",   no_argument,       0, 'n'},
  {0, 0, 0, 0}
};

static void usage(void)
{
  fprintf(stderr, "usage: uhuru-scan [options] FILE|DIR\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Uhuru antivirus scanner\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  --help  -h               print help and quit\n");
  fprintf(stderr, "  --recursive  -r          scan directories recursively\n");
  fprintf(stderr, "  --threaded -t            scan using multiple threads\n");
  fprintf(stderr, "  --no-summary -n          disable summary at end of scanning\n");
  fprintf(stderr, "\n");

  exit(1);
}

static void parse_options(int argc, char *argv[], struct umwsu_scan_options *opts)
{
  int c;

  opts->recursive = 0;
  opts->threaded = 1;
  opts->no_summary = 0;
  opts->path = NULL;

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
    case 't':
      opts->threaded = 1;
      break;
    case 'n':
      opts->no_summary = 1;
      break;
    default:
      usage();
    }
  }

  if (optind < argc)
    opts->path = argv[optind];

  if (opts->path == NULL)
    usage();
}

static void report_print_callback(struct umwsu_report *report, void *callback_data)
{
  FILE *out = (FILE *)callback_data;

  umwsu_report_print(report, out);
}

static void summary_callback(struct umwsu_report *report, void *callback_data)
{
  struct umwsu_scan_summary *s = (struct umwsu_scan_summary *)callback_data;

  s->scanned++;

  switch(report->status) {
  case UMWSU_MALWARE:
    s->malware++;
    break;
  case UMWSU_SUSPICIOUS:
    s->suspicious++;
    break;
  case UMWSU_EINVAL:
  case UMWSU_IERROR:
  case UMWSU_UNKNOWN_FILE_TYPE:
  case UMWSU_UNDECIDED:
    s->unhandled++;
    break;
  case UMWSU_WHITE_LISTED:
  case UMWSU_CLEAN:
    s->clean++;
    break;
  }
}

int main(int argc, char **argv)
{
  struct umwsu *u;
  struct umwsu_scan *scan;
  enum umwsu_scan_flags flags = 0;
  struct umwsu_scan_options opts;
  struct umwsu_scan_summary summary;
  
  parse_options(argc, argv, &opts);

  if (opts.recursive)
    flags |= UMWSU_SCAN_RECURSE;

  if (opts.threaded)
    flags |= UMWSU_SCAN_THREADED;

  u = umwsu_open(0);

  umwsu_set_verbose(u, 1);

#if 0
  umwsu_print(u);
#endif

  scan = umwsu_scan_new(u, opts.path, flags);

  umwsu_scan_add_callback(scan, report_print_callback, stdout);

  if (!opts.no_summary) {
    summary.scanned = summary.malware = summary.suspicious = summary.unhandled = summary.clean = 0;

    umwsu_scan_add_callback(scan, summary_callback, &summary);
  }

  umwsu_scan_start(scan);

  umwsu_scan_run(scan);

  umwsu_scan_free(scan);

  umwsu_close(u);

  if (!opts.no_summary) {
    printf("\nSCAN SUMMARY:\n");
    printf("scanned files     : %d\n", summary.scanned);
    printf("malware files     : %d\n", summary.malware);
    printf("suspicious files  : %d\n", summary.suspicious);
    printf("unhandled files   : %d\n", summary.unhandled);
    printf("clean files       : %d\n", summary.clean);
  }

  return 0;
}
