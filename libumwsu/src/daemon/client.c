#include "client.h"
#include "lib/protocol.h"

#include <stdio.h>
#include <stdlib.h>

struct client {
  int sock;
  struct protocol_handler *handler;
};

static void cb_ping(struct protocol_handler *h, void *data)
{
  struct client *cl = (struct client *)data;

  printf("callback cb_ping\n");

  protocol_handler_output_message(cl->handler, "PONG", NULL);
}

static void cb_stat(struct protocol_handler *h, void *data)
{
  struct client *cl = (struct client *)data;

  printf("callback cb_stat\n");
}

struct client *client_new(int client_sock)
{
  struct client *cl = (struct client *)malloc(sizeof(struct client));

  cl->sock = client_sock;
  cl->handler = protocol_handler_new(cl->sock, cl->sock);

  protocol_handler_add_callback(cl->handler, "PING", cb_ping, cl);
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

  fprintf(stderr, "foo %d %s\n", n_read, buff);

  protocol_handler_input_buffer(cl->handler, buff, n_read);

  return 0;
}
