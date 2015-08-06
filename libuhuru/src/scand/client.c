#include "client.h"
#include "lib/protocol.h"
#include "libuhuru-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

struct client {
  int sock;
  struct protocol_handler *handler;
  struct uhuru *uhuru;
};

static void cb_ping(struct protocol_handler *h, void *data)
{
  struct client *cl = (struct client *)data;

  fprintf(stderr, "callback cb_ping\n");

  protocol_handler_send_msg(cl->handler, "PONG", NULL);
}

static void scan_callback(struct uhuru_report *report, void *callback_data)
{
  struct client *cl = (struct client *)callback_data;
  char status[32], action[32];

  sprintf(status, "%d", report->status);
  sprintf(action, "%d", report->action);
  protocol_handler_send_msg(cl->handler, "SCAN_FILE", 
			    "Path", report->path, 
			    "Status", status,
			    "Module-Name", report->mod_name,
			    "X-Status", report->mod_report,
			    "Action", action,
			    NULL);
}

static void cb_scan(struct protocol_handler *h, void *data)
{
  struct client *cl = (struct client *)data;
  char *path = protocol_handler_get_header(h, "Path");
  struct uhuru_scan *scan;

  fprintf(stderr, "callback cb_scan path: %s\n", path);

  scan = uhuru_scan_new(cl->uhuru, path, UHURU_SCAN_RECURSE);

  uhuru_scan_add_callback(scan, scan_callback, cl);

  uhuru_scan_start(scan);

  while (uhuru_scan_run(scan) == UHURU_SCAN_CONTINUE)
    ;

  uhuru_scan_free(scan);

  protocol_handler_send_msg(cl->handler, "SCAN_END", NULL);

  /* FIXME: should not be there: the UI must close the connexion when processing SCAN_END message, then the server will close */
  close(cl->sock);

  fprintf(stderr, "callback cb_scan end\n");
}

static void cb_stat(struct protocol_handler *h, void *data)
{
  struct client *cl = (struct client *)data;

  fprintf(stderr, "callback cb_stat\n");
}

struct client *client_new(int client_sock, struct uhuru *uhuru)
{
  struct client *cl = (struct client *)malloc(sizeof(struct client));

  cl->sock = client_sock;
  cl->handler = protocol_handler_new(cl->sock, cl->sock);
  cl->uhuru = uhuru;

  protocol_handler_add_callback(cl->handler, "PING", cb_ping, cl);
  protocol_handler_add_callback(cl->handler, "SCAN", cb_scan, cl);
  protocol_handler_add_callback(cl->handler, "STATS", cb_stat, cl);

  return cl;
}

int client_process(struct client *cl)
{
  return protocol_handler_receive(cl->handler);
}
