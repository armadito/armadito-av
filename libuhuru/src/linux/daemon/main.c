#include <libuhuru/core.h>
#include "config/libuhuru-config.h"

#include "utils/getopt.h"
#include "log.h"
#include "server.h"
#include "daemonize.h"
#include "tcpsockserver.h"
#include "unixsockserver.h"
#include "net/netdefaults.h"

#include <glib.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_LOG_LEVEL     "error"
#define DEFAULT_IPC_TYPE      "old"
#define DEFAULT_PID_FILE      LOCALSTATEDIR "/run/uhuru-scand.pid"

#define PROGRAM_NAME "uhuru-scand"
#define PROGRAM_VERSION PACKAGE_VERSION

struct uhuru_daemon_options {
  int no_daemon;
  enum {
    TCP_SOCKET,
    UNIX_SOCKET,
  } socket_type;
  unsigned short port_number;
  const char *unix_path;
  const char *s_log_level;
  const char *pid_file;
  enum ipc_type ipc_type;
};

struct opt daemon_opt_defs[] = {
  { .long_form = "help", .short_form = 'h', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "version", .short_form = 'V', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "no-daemon", .short_form = 'n', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "log-level", .short_form = 'l', .need_arg = 1, .is_set = 0, .value = NULL},
  { .long_form = "tcp", .short_form = 't', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "port", .short_form = 'p', .need_arg = 1, .is_set = 0, .value = NULL},
  { .long_form = "unix", .short_form = 'u', .need_arg = 0, .is_set = 0, .value = NULL},
  { .long_form = "path", .short_form = 'a', .need_arg = 1, .is_set = 0, .value = NULL},
  { .long_form = "pidfile", .short_form = 'i', .need_arg = 1, .is_set = 0, .value = NULL},
  { .long_form = "ipc", .short_form = 'c', .need_arg = 1, .is_set = 0, .value = NULL},
  { .long_form = NULL, .short_form = '\0', .need_arg = 0, .is_set = 0, .value = NULL},
};

static void version(void)
{
  printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
  exit(EXIT_SUCCESS);
}

static void usage(void)
{
  fprintf(stderr, "usage: uhuru-daemon [options]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Uhuru antivirus scanner daemon\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  --help  -h                         print help and quit\n");
  fprintf(stderr, "  --version -V                       print program version\n");
  fprintf(stderr, "  --no-daemon -n                     do not fork and go to background\n");
  fprintf(stderr, "  --log-level=LEVEL | -l LEVEL       set log level\n");
  fprintf(stderr, "                                     log level can be: error, warning, info, debug\n");
  fprintf(stderr, "                                     (default is : " DEFAULT_LOG_LEVEL "\n");
  fprintf(stderr, "  --tcp -t | --unix -u               use TCP (--tcp) or unix (--unix) socket (default is " DEFAULT_SOCKET_TYPE ")\n");
  fprintf(stderr, "  --port=PORT | -p PORT              TCP port number (default is " DEFAULT_SOCKET_PORT ")\n");
  fprintf(stderr, "  --path=PATH | -a PATH              unix socket path (default is " DEFAULT_SOCKET_PATH ")\n");
  fprintf(stderr, "  --pidfile=PATH | -i PATH           create PID file at specified location\n");
  fprintf(stderr, "                                     (default is : " DEFAULT_PID_FILE ")\n");
  fprintf(stderr, "  --ipc=old|json | -c old|json       select IPC type for communication with the user interface\n");
  fprintf(stderr, "                                     json: for new web interface\n");
  fprintf(stderr, "                                     old: for old Qt interface\n");
  fprintf(stderr, "                                     (default is : " DEFAULT_IPC_TYPE ")\n");
  fprintf(stderr, "\n");

  exit(EXIT_FAILURE);
}

static int check_log_level(const char *s_log_level)
{
  return strcmp(s_log_level,"error")
    && strcmp(s_log_level,"warning")
    && strcmp(s_log_level,"info")
    && strcmp(s_log_level,"debug");
}

static int check_ipc_type(const char *s_ipc_type)
{
  return strcmp(s_ipc_type, "old") && strcmp(s_ipc_type, "json");
}

static void parse_options(int argc, const char **argv, struct uhuru_daemon_options *opts)
{
  int r = opt_parse(daemon_opt_defs, argc, argv);
  const char *s_port, *s_ipc_type, *s_socket_type;

  if (r < 0 || r > argc)
    usage();

  if (opt_is_set(daemon_opt_defs, "help"))
      usage();

  if (opt_is_set(daemon_opt_defs, "version"))
      version();

  if (opt_is_set(daemon_opt_defs, "tcp") && opt_is_set(daemon_opt_defs, "unix"))
    usage();

  opts->pid_file = NULL;

  opts->no_daemon = opt_is_set(daemon_opt_defs, "no-daemon");

  opts->s_log_level = opt_value(daemon_opt_defs, "log-level", DEFAULT_LOG_LEVEL);
  if (check_log_level(opts->s_log_level))
    usage();

  s_socket_type = DEFAULT_SOCKET_TYPE;
  if (opt_is_set(daemon_opt_defs, "tcp"))
    s_socket_type = "tcp";
  else if (opt_is_set(daemon_opt_defs, "unix"))
    s_socket_type = "unix";
  opts->socket_type = (!strcmp(s_socket_type, "unix")) ? UNIX_SOCKET : TCP_SOCKET;

  s_port = opt_value(daemon_opt_defs, "port", DEFAULT_SOCKET_PORT);
  opts->port_number = (unsigned short)atoi(s_port);

  opts->unix_path = opt_value(daemon_opt_defs, "path", DEFAULT_SOCKET_PATH);

  if (opt_is_set(daemon_opt_defs, "pidfile"))
    opts->pid_file = opt_value(daemon_opt_defs, "pidfile", DEFAULT_PID_FILE);

  s_ipc_type = opt_value(daemon_opt_defs, "ipc", DEFAULT_IPC_TYPE);
  if (check_ipc_type(s_ipc_type))
    usage();
  opts->ipc_type = (!strcmp(s_ipc_type, "old")) ? OLD_IPC : JSON_IPC;
}

static void create_pid_file(const char *pidfile)
{
  FILE *f = fopen(pidfile, "w");

  if (f == NULL)
    goto err;

  if (fprintf(f, "%d\n", getpid()) < 0)
    goto err;

  if (fclose(f) != 0)
    goto err;

  return;

 err:
  uhuru_log(UHURU_LOG_SERVICE, UHURU_LOG_LEVEL_ERROR, "cannot create PID file %s (errno %d)", pidfile, errno);
  exit(EXIT_FAILURE);
}

static int create_server_socket(struct uhuru_daemon_options *opts)
{
  int server_sock = -1;

  if (opts->socket_type == TCP_SOCKET)
    server_sock = tcp_server_listen(opts->port_number, "127.0.0.1");
  else
    server_sock = unix_server_listen(opts->unix_path);

  if (server_sock < 0) {
    uhuru_log(UHURU_LOG_SERVICE, UHURU_LOG_LEVEL_ERROR, "cannot open server socket (errno %d)", errno);
    exit(EXIT_FAILURE);
  }

  return server_sock;
}

static void start_daemon(const char *progname, struct uhuru_daemon_options *opts)
{
  struct uhuru_conf *conf;
  struct uhuru *uhuru;
  int server_sock;
  struct server *server;
  uhuru_error *error = NULL;
  GMainLoop *loop;

  loop = g_main_loop_new(NULL, FALSE);

  log_init(opts->s_log_level, !opts->no_daemon);

  if (!opts->no_daemon)
    daemonize();

  if (opts->pid_file != NULL)
    create_pid_file(opts->pid_file);

  uhuru_log(UHURU_LOG_SERVICE, UHURU_LOG_LEVEL_NONE, "starting %s%s", progname, opts->no_daemon ? "" : " in daemon mode");

  server_sock = create_server_socket(opts);

  // TODO: loading the configuration
  conf = NULL;
  uhuru = uhuru_open(conf, &error);
  if (uhuru == NULL) {
    uhuru_error_print(error, stderr);
    close(server_sock);
    exit(EXIT_FAILURE);
  }

  server = server_new(uhuru, server_sock, opts->ipc_type);

  g_main_loop_run(loop);
}

int main(int argc, const char **argv)
{
  struct uhuru_daemon_options opts;

#ifdef HAVE_GTHREAD_INIT
  g_thread_init(NULL);
#endif

  parse_options(argc, argv, &opts);

  start_daemon(argv[0], &opts);

  return EXIT_SUCCESS;
}
