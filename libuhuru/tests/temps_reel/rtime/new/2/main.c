#include "monitor.h"
#include "mimetype.h"
#include "pollset.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

struct fanotify_options {
  int enable_permission;
  int mark_mount;
  int check_type;
  int log_event;
};

static void usage(void)
{
  fprintf(stderr, "usage: fanotify-test DIR...\n");
  fprintf(stderr, "fanotify() test\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -h                             print help and quit\n");
  fprintf(stderr, "  -a                             enable permission\n");
  fprintf(stderr, "  -m                             mark the mount points of DIR... (does not work without)\n");
  fprintf(stderr, "  -t                             check file mime type before ALLOWing\n");
  fprintf(stderr, "  -l                             log every fanotify event\n");
  fprintf(stderr, "\n");

  exit(1);
}

static int process_options(int argc, const char **argv, struct fanotify_options *opts)
{
  int argp = 1;

  if (argc < 2)
    usage();

  opts->enable_permission = 0;
  opts->mark_mount = 0;
  opts->check_type = 0;
  opts->log_event = 0;

  while(argp < argc) {
    if (!strcmp(argv[argp], "-h"))
      usage();
    else if (!strcmp(argv[argp], "-a")) {
      opts->enable_permission = 1;
      argp++;
    } else if (!strcmp(argv[argp], "-m")) {
      opts->mark_mount = 1;
      argp++;
    } else if (!strcmp(argv[argp], "-t")) {
      opts->check_type = 1;
      argp++;
    } else if (!strcmp(argv[argp], "-l")) {
      opts->log_event = 1;
      argp++;
    } else
      break;
  }

  return argp;
}

int main(int argc, const char **argv)
{
  struct poll_set *ps;
  struct fanotify_options opts;
  struct access_monitor *monitor;
  enum access_monitor_flags flags = 0;
  int argp;

  argp = process_options(argc, argv, &opts);

  mime_type_init();

  if (opts.mark_mount)
    flags |= MONITOR_MOUNT;

  if (opts.check_type)
    flags |= MONITOR_CHECK_TYPE;

  if (opts.log_event)
    flags |= MONITOR_LOG_EVENT;

  monitor = access_monitor_new(flags);

  if (monitor == NULL)
    return 2;

  access_monitor_enable_permission(monitor, opts.enable_permission);

  while(argp < argc) {
    access_monitor_add(monitor, argv[argp]);
    argp++;
  }

  ps = poll_set_new();

  poll_set_add_fd(ps, access_monitor_get_poll_fd(monitor), access_monitor_cb, monitor);

  access_monitor_activate(monitor);

  return poll_set_loop(ps);
}
