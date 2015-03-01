#include "client.h"
#include "lib/protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

struct client {
  int sock;
  struct protocol_handler *handler;
  struct umwsu *u;
};

static void cb_ping(struct protocol_handler *h, void *data)
{
  struct client *cl = (struct client *)data;

  fprintf(stderr, "callback cb_ping\n");

  protocol_handler_output_message(cl->handler, "PONG", NULL);
}

static void cb_scan(struct protocol_handler *h, void *data)
{
  struct client *cl = (struct client *)data;
  char *path = protocol_handler_header_value(h, "Path");

  fprintf(stderr, "callback cb_scan\n");
  fprintf(stderr, "Path: %s\n", path);

  /* créer struct scan */
  /* ajouter le callback d'envoi */
  /* créer un thread */

  protocol_handler_output_message(cl->handler, "SCAN_FILE", 
				  "Path", "/var/tmp/foo/bar/zob1", 
				  "Status", "OK", 
				  NULL);
  protocol_handler_output_message(cl->handler, "SCAN_FILE", 
				  "Path", "/var/tmp/foo/bar/zob2", 
				  "Status", "MALWARE",
				  "X-Status", "GrosTrojan",
				  "Action", "QUARANTINE",
				  NULL);
  protocol_handler_output_message(cl->handler, "SCAN_END", NULL);
}

static void cb_stat(struct protocol_handler *h, void *data)
{
  struct client *cl = (struct client *)data;

  fprintf(stderr, "callback cb_stat\n");
}

struct client *client_new(int client_sock, struct umwsu *u)
{
  struct client *cl = (struct client *)malloc(sizeof(struct client));

  cl->sock = client_sock;
  cl->handler = protocol_handler_new(cl->sock, cl->sock);
  cl->u = u;

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
