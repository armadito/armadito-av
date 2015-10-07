#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "server.h"
#ifdef linux
#include "os/linux/daemonize.h"
#endif

#include <getopt.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

struct uhuru_daemon_options {
  int no_daemon;
  /* more options later */
};

static struct option long_options[] = {
  {"help",         no_argument,       0, 'h'},
  {"no-daemon",    no_argument,       0, 'n'},
  {0, 0, 0, 0}
};

static void usage(void)
{
  fprintf(stderr, "usage: uhuru-daemon [options]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Uhuru antivirus scanner daemon\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  --help  -h               print help and quit\n");
  fprintf(stderr, "  --no-daemon -n           do not fork and go to background\n");
  fprintf(stderr, "\n");

  exit(1);
}

static void parse_options(int argc, char *argv[], struct uhuru_daemon_options *opts)
{
  int c;

  opts->no_daemon = 0;

  while (1) {
    int option_index = 0;

    c = getopt_long(argc, argv, "hn", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 'h':
      usage();
      break;
    case 'n':
      opts->no_daemon = 1;
      break;
    default:
      usage();
    }
  }
}

int main(int argc, char **argv)
{
  struct uhuru_daemon_options opts;

  g_thread_init(NULL);
  g_type_init();

  parse_options(argc, argv, &opts);

  if (!opts.no_daemon)
    daemonize();

  server_loop(server_new(0));

  return 0;
}
