#include <libuhuru/scan.h>
#include "lib/statusp.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

struct scan_summary {
  int scanned;
  int malware;
  int suspicious;
  int unhandled;
  int clean;
};

struct scan_options {
  int use_daemon;
  int recursive;
  int threaded;
  int no_summary;
  int print_clean;
  /* more options later */
  char *path;
};

static struct option long_options[] = {
  {"help",         no_argument,       0, 'h'},
  {"local",        no_argument,       0, 'l'},
  {"recursive",    no_argument,       0, 'r'},
  {"threaded",     no_argument,       0, 't'},
  {"no-summary",   no_argument,       0, 'n'},
  {"print-clean",  no_argument,       0, 'c'},
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
  fprintf(stderr, "  --local  -l              do not use the scan daemon\n");
  fprintf(stderr, "  --recursive  -r          scan directories recursively\n");
  fprintf(stderr, "  --threaded -t            scan using multiple threads\n");
  fprintf(stderr, "  --no-summary -n          disable summary at end of scanning\n");
  fprintf(stderr, "  --print-clean -c         print also clean files as they are scanned\n");
  fprintf(stderr, "\n");

  exit(1);
}

static void parse_options(int argc, char *argv[], struct scan_options *opts)
{
  int c;

  opts->use_daemon = 1;
  opts->recursive = 0;
  opts->threaded = 1;
  opts->no_summary = 0;
  opts->print_clean = 0;
  opts->path = NULL;

  while (1) {
    int option_index = 0;

    c = getopt_long(argc, argv, "hlrtnc", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 'h':
      usage();
      break;
    case 'l':
      opts->use_daemon = 0;
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
    case 'c':
      opts->print_clean = 1;
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

static void report_print(struct uhuru_report *report, FILE *out)
{
  fprintf(out, "%s: %s", report->path, uhuru_file_status_pretty_str(report->status));
  if (report->status != UHURU_UNDECIDED && report->status != UHURU_CLEAN && report->status != UHURU_UNKNOWN_FILE_TYPE)
    fprintf(out, " [%s - %s]", report->mod_name, report->mod_report);
  if (report->action != UHURU_ACTION_NONE)
    fprintf(out, " (action %s)", uhuru_action_pretty_str(report->action));
  fprintf(out, "\n");
}

static void report_print_callback(struct uhuru_report *report, void *callback_data)
{
  int *print_clean = (int *)callback_data;

  if (!*print_clean && (report->status == UHURU_WHITE_LISTED || report->status == UHURU_CLEAN))
    return;

  report_print(report, stdout);
}

static void summary_callback(struct uhuru_report *report, void *callback_data)
{
  struct scan_summary *s = (struct scan_summary *)callback_data;

  s->scanned++;

  switch(report->status) {
  case UHURU_MALWARE:
    s->malware++;
    break;
  case UHURU_SUSPICIOUS:
    s->suspicious++;
    break;
  case UHURU_EINVAL:
  case UHURU_IERROR:
  case UHURU_UNKNOWN_FILE_TYPE:
  case UHURU_UNDECIDED:
    s->unhandled++;
    break;
  case UHURU_WHITE_LISTED:
  case UHURU_CLEAN:
    s->clean++;
    break;
  }
}

static void poll_add_fd(int epoll_fd, int fd)
{
  struct epoll_event ev;

  ev.events = EPOLLIN;
  ev.data.fd = fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }
}

static void scan_loop_poll(struct uhuru_scan *scan)
{
#define MAX_EVENTS 10
  struct epoll_event events[MAX_EVENTS];
  int epoll_fd;

  epoll_fd = epoll_create(42);
  if (epoll_fd < 0) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }

  poll_add_fd(epoll_fd, STDIN_FILENO);
  poll_add_fd(epoll_fd, uhuru_scan_get_poll_fd(scan));

  while (1) {
    int n_ev, n;

    n_ev = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (n_ev == -1) {
      perror("epoll_wait");
      exit(EXIT_FAILURE);
    }

    for (n = 0; n < n_ev; n++) {
      if (events[n].data.fd == STDIN_FILENO) {
	;
      } else {
	if (uhuru_scan_run(scan) != UHURU_SCAN_CONTINUE)
	  return;
      }
    }
  }
}

static void scan_loop_no_poll(struct uhuru_scan *scan)
{
  while(uhuru_scan_run(scan) == UHURU_SCAN_CONTINUE)
    ;
}

static void do_scan(struct scan_options *opts, struct scan_summary *summary)
{
  enum uhuru_scan_flags flags = 0;
  struct uhuru *u;
  struct uhuru_scan *scan;

  if (opts->recursive)
    flags |= UHURU_SCAN_RECURSE;

  if (opts->threaded)
    flags |= UHURU_SCAN_THREADED;

  u = uhuru_open(opts->use_daemon);

#if 0
  uhuru_print(u);
#endif

  scan = uhuru_scan_new(u, opts->path, flags);

  uhuru_scan_add_callback(scan, report_print_callback, &opts->print_clean);

  if (!opts->no_summary) {
    summary->scanned = summary->malware = summary->suspicious = summary->unhandled = summary->clean = 0;

    uhuru_scan_add_callback(scan, summary_callback, summary);
  }

  uhuru_scan_start(scan);

  if (opts->use_daemon)
    scan_loop_poll(scan);
  else
    scan_loop_no_poll(scan);

  uhuru_scan_free(scan);

  uhuru_close(u);
}

static void print_summary(struct scan_summary *summary)
{
  printf("\nSCAN SUMMARY:\n");
  printf("scanned files     : %d\n", summary->scanned);
  printf("malware files     : %d\n", summary->malware);
  printf("suspicious files  : %d\n", summary->suspicious);
  printf("unhandled files   : %d\n", summary->unhandled);
  printf("clean files       : %d\n", summary->clean);
}

int main(int argc, char **argv)
{
  struct scan_options *opts = (struct scan_options *)malloc(sizeof(struct scan_options));
  struct scan_summary *summary = NULL;

  parse_options(argc, argv, opts);

  if (!opts->no_summary)
    summary = (struct scan_summary *)malloc(sizeof(struct scan_summary));

  do_scan(opts, summary);

  if (!opts->no_summary)
    print_summary(summary);

  return 0;
}
