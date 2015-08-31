#include "client.h"
#include "lib/ipc.h"
#include "libuhuru-config.h"

#include <stdio.h>
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

  fprintf(stderr, "callback cb_ping\n");

  ipc_manager_send_msg(cl->manager, IPC_MSG_ID_PONG, IPC_NONE);
}

static void scan_callback(struct uhuru_report *report, void *callback_data)
{
  struct client *cl = (struct client *)callback_data;
  char status[32], action[32];

  sprintf(status, "%d", report->status);
  sprintf(action, "%d", report->action);
  ipc_manager_send_msg(cl->manager, IPC_MSG_ID_SCAN_FILE, 
		       IPC_STRING, report->path, 
		       IPC_STRING, status,
		       IPC_STRING, report->mod_name,
		       IPC_STRING, report->mod_report,
		       IPC_STRING, action,
		       IPC_NONE);
}

static void scan_handler(struct ipc_manager *m, void *data)
{
  struct client *cl = (struct client *)data;
  char *path;
  struct uhuru_scan *scan;

  ipc_manager_get_arg_at(m, 0, IPC_STRING, &path);
  fprintf(stderr, "callback cb_scan path: %s\n", path);

  scan = uhuru_scan_new(cl->uhuru, path, UHURU_SCAN_RECURSE);

  uhuru_scan_add_callback(scan, scan_callback, cl);

  uhuru_scan_start(scan);

  while (uhuru_scan_run(scan) == UHURU_SCAN_CONTINUE)
    ;

  uhuru_scan_free(scan);

  ipc_manager_send_msg(cl->manager, IPC_MSG_ID_SCAN_END, IPC_NONE);

  /* FIXME: should not be there: the UI must close the connexion when processing SCAN_END message, then the server will close */
  close(cl->sock);

  fprintf(stderr, "callback cb_scan end\n");
}

static void cb_stat(struct ipc_manager *m, void *data)
{
  struct client *cl = (struct client *)data;

  fprintf(stderr, "callback cb_stat\n");
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

int client_process(struct client *cl)
{
  return ipc_manager_receive(cl->manager);
}
