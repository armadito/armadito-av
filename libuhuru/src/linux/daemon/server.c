#include <libuhuru/core.h>

#include "server.h"
#include "ipcclient.h"
#include "jsonclient.h"

#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

struct server {
  int listen_sock;
  struct uhuru *uhuru;
  GThreadPool *thread_pool;  
  GIOChannel *channel;
  enum ipc_type ipc_type;
};

static void client_thread(gpointer data, gpointer user_data)
{
  struct server *server = (struct server *)user_data;

  switch(server->ipc_type) {
  case OLD_IPC: 
    {
      struct ipc_client *client = (struct ipc_client *)data;
      
      while (ipc_client_process(client) > 0)
	;
      
      ipc_client_free(client);
    }
    break;
  case JSON_IPC:
    {
      struct json_client *client = (struct json_client *)data;
      
      json_client_process(client);
      
      json_client_free(client);
    }
    break;
  }
}

static gboolean server_listen_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct server *server = (struct server *)data;
  void *client;
  int client_sock;

  client_sock = accept(server->listen_sock, NULL, NULL);

  if (client_sock < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "accept() failed (%s)", strerror(errno));
    return FALSE;
  }

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "accepted client connection: fd = %d", client_sock);

  switch(server->ipc_type) {
  case OLD_IPC:
    client = ipc_client_new(client_sock, server->uhuru);
    break;
  case JSON_IPC:
    client = json_client_new(client_sock, server->uhuru);
    break;
  }

  g_thread_pool_push(server->thread_pool, (gpointer)client, NULL);

  return TRUE;
}

struct server *server_new(struct uhuru *uhuru, int server_sock, enum ipc_type ipc_type)
{
  struct server *server = (struct server *)malloc(sizeof(struct server));
  assert(server != NULL);

  server->uhuru = uhuru;
  server->listen_sock = server_sock;
  server->ipc_type = ipc_type;

  server->thread_pool = g_thread_pool_new(client_thread, server, -1, FALSE, NULL);

  server->channel = g_io_channel_unix_new(server->listen_sock);
  g_io_add_watch(server->channel, G_IO_IN, server_listen_cb, server);

  return server;
}
