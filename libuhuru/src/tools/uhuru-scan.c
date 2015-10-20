#include <libuhuru/ipc.h>

#include "utils/getopt.h"
#include "utils/tcpsock.h"
#include "utils/named_pipe_server.h"

#include <assert.h>
#include <errno.h>
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
  int recursive;
  int threaded;
  int no_summary;
  int print_clean;
  int use_tcp;
  unsigned short port_number;
  const char *path;
};

struct opt scan_opt_defs[] = {
  { .long_form = "help", .short_form = 'h', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "recursive", .short_form = 'r', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "threaded", .short_form = 't', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "no-summary", .short_form = 'n', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "print-clean", .short_form = 'c', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "tcp", .short_form = 't', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "port", .short_form = 'p', .need_arg = 1, .is_set = 0, .value = NULL},
  { .long_form = NULL, .short_form = '\0', .need_arg = 0, .is_set = 0, .value = NULL},
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
  fprintf(stderr, "  --print-clean -c         print also clean files as they are scanned\n");
  fprintf(stderr, "  --tcp -t                 use TCP socket\n");
  fprintf(stderr, "  --port -p                TCP port number\n");
  fprintf(stderr, "\n");

  exit(1);
}

static void parse_options(int argc, const char *argv[], struct scan_options *opts)
{
  int r = opt_parse(scan_opt_defs, argc, argv);
  const char *s_port;

  if (r < 0|| r >= argc)
    usage();

  opts->recursive = opt_is_set(scan_opt_defs, "recursive");
  opts->threaded = opt_is_set(scan_opt_defs, "threaded");
  opts->no_summary = opt_is_set(scan_opt_defs, "no-summary");
  opts->print_clean = opt_is_set(scan_opt_defs, "print-clean");
  opts->use_tcp = opt_is_set(scan_opt_defs, "tcp");
  s_port = opt_value(scan_opt_defs, "port", "14444");
  opts->port_number = (unsigned short)atoi(s_port);

  opts->path = argv[r];
}

static const char *file_status_pretty_str(enum uhuru_file_status status)
{
  switch(status) {
  case UHURU_UNDECIDED:
    return "undecided";
  case UHURU_CLEAN:
    return "clean";
  case UHURU_UNKNOWN_FILE_TYPE:
    return "ignored";
  case UHURU_EINVAL:
    return "invalid argument";
  case UHURU_IERROR:
    return "internal error";
  case UHURU_SUSPICIOUS:
    return "suspicious";
  case UHURU_WHITE_LISTED:
    return "white listed";
  case UHURU_MALWARE:
    return "malware";
  }

  return "unknown status";
}

static const char *action_pretty_str(enum uhuru_action action)
{
  switch(action & (UHURU_ACTION_ALERT | UHURU_ACTION_QUARANTINE | UHURU_ACTION_REMOVE)) {
  case UHURU_ACTION_ALERT: return "alert";
  case UHURU_ACTION_ALERT | UHURU_ACTION_QUARANTINE: return "alert+quarantine";
  case UHURU_ACTION_ALERT | UHURU_ACTION_REMOVE: return "alert+removed";
  }

  return "none";
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

  printf("%s: %s", path, file_status_pretty_str(status));
  if (status != UHURU_UNDECIDED && status != UHURU_CLEAN && status != UHURU_UNKNOWN_FILE_TYPE)
    printf(" [%s - %s]", mod_name, x_status);
  if (action != UHURU_ACTION_NONE)
    printf(" (action %s)", action_pretty_str(action));
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

static void do_scan(struct scan_options *opts, int client_sock)
{
  struct scan *scan;
  struct ipc_manager *manager;

  scan = (struct scan *)malloc(sizeof(struct scan));
  assert(scan != NULL);

  scan->summary = (opts->no_summary) ? NULL : (struct scan_summary *)malloc(sizeof(struct scan_summary));
  scan->print_clean = opts->print_clean;

  manager = ipc_manager_new(client_sock);

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

int main(int argc, const char **argv)
{
  struct scan_options *opts = (struct scan_options *)malloc(sizeof(struct scan_options));
  int named_pipe = 0;

  //parse_options(argc, argv, opts);

  // Notes : If you intend to use a named pipe locally only, deny access to NT AUTHORITY\NETWORK or switch to local RPC.

  named_pipe = start_named_pipe_server();

  /*
  client_sock = tcp_client_connect("127.0.0.1", opts->port_number, 10);

  if (client_sock < 0) {
    fprintf(stderr, "cannot open client socket (errno %d)\n", errno);
    return 1;
  }
  */

  // do_scan(opts, client_sock);

  return 0;
}
