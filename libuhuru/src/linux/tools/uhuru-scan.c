#include "config/libuhuru-config.h"

#include "utils/getopt.h"
#include "daemon/ipc.h"
#include "net/tcpsockclient.h"
#include "net/unixsockclient.h"
#include "net/netdefaults.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_NAME "uhuru-scan"
#define PROGRAM_VERSION PACKAGE_VERSION

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
  enum {
    TCP_SOCKET,
    UNIX_SOCKET,
  } socket_type;
  const char *unix_path;
  unsigned short port_number;
  int recursive;
  int threaded;
  int no_summary;
  int print_clean;
  const char *path;
};

static struct opt scan_opt_defs[] = {
  { "help", 'h', 0, 0, NULL},
  { "recursive", 'r', 0, 0, NULL},
  { "threaded", 't', 0, 0, NULL},
  { "no-summary", 'n',  0, 0, NULL},
  { "print-clean", 'c', 0, 0, NULL},
  { .long_form = "version", .short_form = 'V', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "tcp", .short_form = 't', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "port", .short_form = 'p', .need_arg = 1, .is_set = 0, .value = NULL},
  { .long_form = "unix", .short_form = 'u', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "path", .short_form = 'a', .need_arg = 1, .is_set = 0, .value = NULL},
  { .long_form = NULL, .short_form = 0, .need_arg = 0, .is_set = 0, .value = NULL},
};

static void version(void)
{
  printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
  exit(EXIT_SUCCESS);
}

static void usage(void)
{
  fprintf(stderr, "usage: uhuru-scan [options] FILE|DIR\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Uhuru antivirus scanner\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  --help  -h                    print help and quit\n");
  fprintf(stderr, "  --version -V                  print program version\n");
  fprintf(stderr, "  --recursive  -r               scan directories recursively\n");
  fprintf(stderr, "  --threaded -t                 scan using multiple threads\n");
  fprintf(stderr, "  --no-summary -n               disable summary at end of scanning\n");
  fprintf(stderr, "  --print-clean -c              print also clean files as they are scanned\n");
  fprintf(stderr, "  --tcp -t | --unix -u          use TCP (--tcp) or unix (--unix) socket (default is " DEFAULT_SOCKET_TYPE ")\n");
  fprintf(stderr, "  --port=PORT | -p PORT         TCP port number (default is " DEFAULT_SOCKET_PORT ")\n");
  fprintf(stderr, "  --path=PATH | -a PATH         unix socket path (default is " DEFAULT_SOCKET_PATH ")\n");
  fprintf(stderr, "\n");

  exit(1);
}

static void parse_options(int argc, const char *argv[], struct scan_options *opts)
{
  int r = opt_parse(scan_opt_defs, argc, argv);
  const char *s_port, *s_socket_type;

  if (r < 0|| r > argc)
    usage();

  if (opt_is_set(scan_opt_defs, "help"))
      usage();

  if (opt_is_set(scan_opt_defs, "version"))
      version();

  if (opt_is_set(scan_opt_defs, "tcp") && opt_is_set(scan_opt_defs, "unix"))
    usage();

  s_socket_type = DEFAULT_SOCKET_TYPE;
  if (opt_is_set(scan_opt_defs, "tcp"))
    s_socket_type = "tcp";
  else if (opt_is_set(scan_opt_defs, "unix"))
    s_socket_type = "unix";
  opts->socket_type = (!strcmp(s_socket_type, "unix")) ? UNIX_SOCKET : TCP_SOCKET;

  s_port = opt_value(scan_opt_defs, "port", DEFAULT_SOCKET_PORT);
  opts->port_number = (unsigned short)atoi(s_port);

  opts->unix_path = opt_value(scan_opt_defs, "path", DEFAULT_SOCKET_PATH);

  opts->recursive = opt_is_set(scan_opt_defs, "recursive");
  opts->threaded = opt_is_set(scan_opt_defs, "threaded");
  opts->no_summary = opt_is_set(scan_opt_defs, "no-summary");
  opts->print_clean = opt_is_set(scan_opt_defs, "print-clean");

  opts->path = argv[r];
}

static void ipc_handler_scan_file(struct ipc_manager *m, void *data)
{
  struct scan *scan = (struct scan *)data;
  char *path, *status, *mod_name, *mod_report, *action;
  int progress, clean = 0;

  ipc_manager_get_arg_at(m, 0, IPC_STRING_T, &path);
  ipc_manager_get_arg_at(m, 1, IPC_STRING_T, &status);
  ipc_manager_get_arg_at(m, 2, IPC_STRING_T, &mod_name);
  ipc_manager_get_arg_at(m, 3, IPC_STRING_T, &mod_report);
  ipc_manager_get_arg_at(m, 4, IPC_STRING_T, &action);
  ipc_manager_get_arg_at(m, 5, IPC_INT32_T, &progress);

  /* path is empty string, do nothing */
  if (!*path)
    return;

  if (scan->summary != NULL) {
    scan->summary->scanned++;

    if (!strcmp(status, "UHURU_MALWARE")) 
      scan->summary->malware++;
    else if (!strcmp(status, "UHURU_SUSPICIOUS"))
      scan->summary->suspicious++;
    else if (!strcmp(status, "UHURU_EINVAL") 
	     || !strcmp(status, "UHURU_IERROR")
	     || !strcmp(status, "UHURU_UNKNOWN_FILE_TYPE")
	     || !strcmp(status, "UHURU_UNDECIDED"))
      scan->summary->unhandled++;
    else if (!strcmp(status, "UHURU_WHITE_LISTED")
	     || !strcmp(status, "UHURU_CLEAN")) {
      scan->summary->clean++;
      clean = 1;
    }
  }

  if (!scan->print_clean && clean)
    return;

  printf("%s: %s", path, status);
  /* if (status != UHURU_UNDECIDED && status != UHURU_CLEAN && status != UHURU_UNKNOWN_FILE_TYPE) */
  printf(" [%s - %s]", mod_name, mod_report);
  printf(" (action %s) ", action);
  printf("[%d]\n", progress);
}

static void ipc_handler_scan_end(struct ipc_manager *m, void *data)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)data;

  /* ??? */
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

  ipc_manager_msg_send(manager, 
		       IPC_MSG_ID_SCAN, 
		       IPC_STRING_T, opts->path, 
		       IPC_INT32_T, opts->threaded, 
		       IPC_INT32_T, opts->recursive, 
		       IPC_NONE_T);

  while (ipc_manager_receive(manager) > 0)
    ;

  if (!opts->no_summary) {
    print_summary(scan->summary);
    free(scan->summary);
  }

  ipc_manager_free(manager);
}

int main(int argc, const char **argv)
{
  struct scan_options *opts = (struct scan_options *)malloc(sizeof(struct scan_options));
  int client_sock;

  parse_options(argc, argv, opts);

  if (opts->socket_type == TCP_SOCKET)
    client_sock = tcp_client_connect("127.0.0.1", opts->port_number, 10);
  else
    client_sock = unix_client_connect(opts->unix_path, 10);

  if (client_sock < 0) {
    fprintf(stderr, "cannot open client socket (errno %d)\n", errno);
    return 1;
  }

  do_scan(opts, client_sock);

  return 0;
}
