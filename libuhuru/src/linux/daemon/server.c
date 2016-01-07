#include <libuhuru/core.h>

#include "server.h"
#include "client.h"

#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(linux)
#include <sys/types.h>
#include <sys/socket.h>
#elif defined(WIN32)
#include <WinSock2.h>
#endif

struct server {
  int listen_sock;
  struct uhuru *uhuru;
  GThreadPool *thread_pool;  
  GIOChannel *channel;
};

static void client_thread(gpointer data, gpointer user_data)
{
  struct client *client = (struct client *)data;

  while (client_process(client) > 0)
    ;

  client_free(client);
}

static gboolean server_listen_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct server *server = (struct server *)data;
  struct client *client;
  int client_sock;

  client_sock = accept(server->listen_sock, NULL, NULL);

  if (client_sock < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "accept() failed: errno = %d", errno);
    return FALSE;
  }

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "accepted client connection: fd = %d", client_sock);

  client = client_new(client_sock, server->uhuru);

  g_thread_pool_push(server->thread_pool, (gpointer)client, NULL);

  return TRUE;
}

struct server *server_new(struct uhuru *uhuru, int server_sock)
{
  struct server *server = (struct server *)malloc(sizeof(struct server));
  assert(server != NULL);

  server->uhuru = uhuru;
  server->listen_sock = server_sock;

  server->thread_pool = g_thread_pool_new(client_thread, server, -1, FALSE, NULL);

  server->channel = g_io_channel_unix_new(server->listen_sock);
  g_io_add_watch(server->channel, G_IO_IN, server_listen_cb, server);

  return server;
}
