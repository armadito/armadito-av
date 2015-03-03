#include "client.h"
#include "lib/protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

struct client {
  int sock;
  struct protocol_handler *handler;
  struct umwsu *umwsu;
};

static void cb_ping(struct protocol_handler *h, void *data)
{
  struct client *cl = (struct client *)data;

  fprintf(stderr, "callback cb_ping\n");

  protocol_handler_output_message(cl->handler, "PONG", NULL);
}

static void scan_callback(struct umwsu_report *report, void *callback_data)
{
  struct client *cl = (struct client *)callback_data;
  char status[32], action[32];

  switch(report->status) {
  case UMWSU_WHITE_LISTED:
  case UMWSU_CLEAN:
    return;
  }

  sprintf(status, "%d", report->status);
  sprintf(action, "%d", report->action);
  protocol_handler_output_message(cl->handler, "SCAN_FILE", 
				  "Path", report->path, 
				  "Status", status,
				  "X-Status", report->mod_report,
				  "Action", action,
				  NULL);
}

struct scan_thread_args {
  char *path;
  struct client *cl;
};

static gpointer scan_thread_fun(gpointer data)
{
  struct scan_thread_args *args = (struct scan_thread_args *)data;
  char *path = args->path;
  struct client *cl = args->cl;
  struct umwsu_scan *scan;

  scan = umwsu_scan_new(cl->umwsu, path, UMWSU_SCAN_RECURSE);

  umwsu_scan_add_callback(scan, scan_callback, cl);

  umwsu_scan_start(scan);

  umwsu_scan_finish(scan);

  umwsu_scan_free(scan);

  protocol_handler_output_message(cl->handler, "SCAN_END", NULL);

  close(cl->sock);

  free(args);
  free(path);

  g_thread_unref(g_thread_self());
}

static void cb_scan(struct protocol_handler *h, void *data)
{
  struct client *cl = (struct client *)data;
  char *path = protocol_handler_header_value(h, "Path");
  struct scan_thread_args *args;
  GThread *scan_thread;

  fprintf(stderr, "callback cb_scan\n");
  fprintf(stderr, "Path: %s\n", path);

  args = (struct scan_thread_args *)malloc(sizeof(struct scan_thread_args));
  args->path = strdup(path);
  args->cl = cl;

  scan_thread = g_thread_new("scan_thread", scan_thread_fun, args);
}

static void cb_stat(struct protocol_handler *h, void *data)
{
  struct client *cl = (struct client *)data;

  fprintf(stderr, "callback cb_stat\n");
}

struct client *client_new(int client_sock, struct umwsu *umwsu)
{
  struct client *cl = (struct client *)malloc(sizeof(struct client));

  cl->sock = client_sock;
  cl->handler = protocol_handler_new(cl->sock, cl->sock);
  cl->umwsu = umwsu;

  protocol_handler_add_callback(cl->handler, "PING", cb_ping, cl);
  protocol_handler_add_callback(cl->handler, "SCAN", cb_scan, cl);
  protocol_handler_add_callback(cl->handler, "STATS", cb_stat, cl);

  return cl;
}

int client_process(struct client *cl)
{
  char buff[100];
  int n_read;

  n_read = read(cl->sock, buff, 100);
  if (n_read == 0)
    return -1;

  protocol_handler_input_buffer(cl->handler, buff, n_read);

  return 0;
}
