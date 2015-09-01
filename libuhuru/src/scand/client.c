#include "client.h"
#include "lib/ipc.h"
#include "libuhuru-config.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

struct client {
  int sock;
  struct ipc_manager *manager;
  struct uhuru *uhuru;
};

static void ping_handler(struct ipc_manager *m, void *data)
{
  struct client *cl = (struct client *)data;

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "ping_handler");
#endif

  ipc_manager_send_msg(cl->manager, IPC_MSG_ID_PONG, IPC_NONE);
}

static void scan_callback(struct uhuru_report *report, void *callback_data)
{
  struct client *cl = (struct client *)callback_data;

  ipc_manager_send_msg(cl->manager, IPC_MSG_ID_SCAN_FILE, 
		       IPC_STRING, report->path, 
		       IPC_INT32, report->status,
		       IPC_STRING, report->mod_name,
		       IPC_STRING, report->mod_report,
		       IPC_INT32, report->action,
		       IPC_NONE);
}

static void scan_handler(struct ipc_manager *m, void *data)
{
  struct client *cl = (struct client *)data;
  char *path;
  struct uhuru_scan *scan;

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "scan_handler");
#endif

  ipc_manager_get_arg_at(m, 0, IPC_STRING, &path);

  scan = uhuru_scan_new(cl->uhuru, path, UHURU_SCAN_RECURSE);

  uhuru_scan_add_callback(scan, scan_callback, cl);

  uhuru_scan_start(scan);

  while (uhuru_scan_run(scan) == UHURU_SCAN_CONTINUE)
    ;

  uhuru_scan_free(scan);

  ipc_manager_send_msg(cl->manager, IPC_MSG_ID_SCAN_END, IPC_NONE);

  if (close(cl->sock) < 0) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "closing socket %d failed (%s)", cl->sock, strerror(errno));
  }

  cl->sock = -1;

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "scan_handler finished");
#endif
}

static void cb_stat(struct ipc_manager *m, void *data)
{
  struct client *cl = (struct client *)data;

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "stat_handler");
#endif
}

struct client *client_new(int client_sock, struct uhuru *uhuru)
{
  struct client *cl = (struct client *)malloc(sizeof(struct client));

  cl->sock = client_sock;
  cl->manager = ipc_manager_new(cl->sock, cl->sock);
  cl->uhuru = uhuru;

  ipc_manager_add_handler(cl->manager, IPC_MSG_ID_PING, ping_handler, cl);
  ipc_manager_add_handler(cl->manager, IPC_MSG_ID_SCAN, scan_handler, cl);

  return cl;
}

void client_free(struct client *cl)
{
  ipc_manager_free(cl->manager);

  free(cl);
}

int client_process(struct client *cl)
{
  int ret;

  if (cl->sock < 0)
    return 0;

  ret = ipc_manager_receive(cl->manager);

  return ret;
}
