#include <libuhuru/ipc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct scan_summary {
  int scanned;
  int malware;
  int suspicious;
  int unhandled;
  int clean;
};

struct scan {
  struct scan_summary *summary;
  int print_clean;
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

static struct optdef {
  const char *long_form;
  char short_form;
} scan_optdefs[] = {
  {"help",         'h'},
  {"local",        'l'},
  {"recursive",    'r'},
  {"threaded",     't'},
  {"no-summary",   'n'},
  {"print-clean",  'c'},
  {NULL, '\0'}
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

static char get_option(const char *argv, struct optdef *od)
{
  while(od->long_form != NULL) {
    if (argv[0] == '-' && argv[1] == '-' && !strcmp(argv + 2, od->long_form))
      return od->short_form;
    else if (argv[0] == '-' && argv[1] == od->short_form)
      return od->short_form;

    od++;
  }

  return -1;
}

static void parse_options(int argc, char *argv[], struct scan_options *scan_opts)
{
  int optind;

  scan_opts->use_daemon = 1;
  scan_opts->recursive = 0;
  scan_opts->threaded = 1;
  scan_opts->no_summary = 0;
  scan_opts->print_clean = 0;
  scan_opts->path = NULL;

  for(optind = 1; optind < argc; optind++) {
    int c;

    c = get_option(argv[optind], scan_optdefs);

    if (c == -1)
      break;

    switch (c) {
    case 'h':
      usage();
      break;
    case 'l':
      scan_opts->use_daemon = 0;
      break;
    case 'r':
      scan_opts->recursive = 1;
      break;
    case 't':
      scan_opts->threaded = 1;
      break;
    case 'n':
      scan_opts->no_summary = 1;
      break;
    case 'c':
      scan_opts->print_clean = 1;
      break;
    default:
      usage();
    }
  }

  if (optind < argc)
    scan_opts->path = argv[optind];

  if (scan_opts->path == NULL)
    usage();
}

static void ipc_handler_scan_file(struct ipc_manager *m, void *data)
{
  struct scan *scan = (struct scan *)data;
  char *path;
  gint32 i_status;
  enum uhuru_file_status status;
  char *mod_name;
  char *x_status;
  gint32 i_action;
  enum uhuru_action action;

  ipc_manager_get_arg_at(m, 0, IPC_STRING_T, &path);
  ipc_manager_get_arg_at(m, 1, IPC_INT32_T, &i_status);
  ipc_manager_get_arg_at(m, 2, IPC_STRING_T, &mod_name);
  ipc_manager_get_arg_at(m, 3, IPC_STRING_T, &x_status);
  ipc_manager_get_arg_at(m, 4, IPC_INT32_T, &i_action);

  status = (enum uhuru_file_status)i_status;
  action = (enum uhuru_action)i_action;

  if (scan->summary != NULL) {
    scan->summary->scanned++;

    switch(status) {
    case UHURU_MALWARE:
      scan->summary->malware++;
      break;
    case UHURU_SUSPICIOUS:
      scan->summary->suspicious++;
      break;
    case UHURU_EINVAL:
    case UHURU_IERROR:
    case UHURU_UNKNOWN_FILE_TYPE:
    case UHURU_UNDECIDED:
      scan->summary->unhandled++;
      break;
    case UHURU_WHITE_LISTED:
    case UHURU_CLEAN:
      scan->summary->clean++;
      break;
    }
  }

  if (!scan->print_clean && (status == UHURU_WHITE_LISTED || status == UHURU_CLEAN))
    return;

  printf("%s: %s", path, uhuru_file_status_pretty_str(status));
  if (status != UHURU_UNDECIDED && status != UHURU_CLEAN && status != UHURU_UNKNOWN_FILE_TYPE)
    printf(" [%s - %s]", mod_name, x_status);
  if (action != UHURU_ACTION_NONE)
    printf(" (action %s)", uhuru_action_pretty_str(action));
  printf("\n");
}

static void ipc_handler_scan_end(struct ipc_manager *m, void *data)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)data;

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "remote scan end");
#endif
  
#if 0
#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "remote scan end, closing socket %d", scan->remote.connect_fd);
#endif
  
  if (close(scan->remote.connect_fd) < 0) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "closing socket %d failed (%s)", scan->remote.connect_fd, os_strerror(errno));
  }

  scan->remote.connect_fd = -1;
#endif
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

static void do_scan(struct scan_options *opts)
{
  struct scan *scan;
  struct ipc_manager *manager;

  scan = (struct scan *)malloc(sizeof(struct scan));
  assert(scan != NULL);

  scan->summary = (opts->no_summary) ? NULL : (struct scan_summary *)malloc(sizeof(struct scan_summary));
  scan->print_clean = opts->print_clean;

  connect_fd = os_ipc_connect(connect_url, 10);
  if (connect_fd < 0)
    return;

  manager = ipc_manager_new(connect_fd);

  ipc_manager_add_handler(manager, IPC_MSG_ID_SCAN_FILE, ipc_handler_scan_file, scan);
  ipc_manager_add_handler(manager, IPC_MSG_ID_SCAN_END, ipc_handler_scan_end, scan);

  ipc_manager_msg_send(manager, IPC_MSG_ID_SCAN, IPC_STRING_T, opts->path, IPC_NONE_T);

  while (ipc_manager_receive(manager) > 0)
    ;

  if (!opts->no_summary) {
    print_summary(scan->summary);
    free(scan->summary);
  }
}

int main(int argc, char **argv)
{
  struct scan_options *opts = (struct scan_options *)malloc(sizeof(struct scan_options));

  parse_options(argc, argv, opts);

  do_scan(opts);

  return 0;
}
