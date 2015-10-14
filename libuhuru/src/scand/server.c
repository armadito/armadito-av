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
  GIOChannel *channel;
  GThreadPool *thread_pool;  
  struct uhuru *uhuru;
};

static void client_thread(gpointer data, gpointer user_data)
{
  struct client *client = (struct client *)data;

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "client thread started");
#endif

  while (client_process(client) > 0)
    ;

  client_free(client);

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "client thread terminated");
#endif
}

static gboolean server_listen_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct server *server = (struct server *)data;
  struct client *client;
  int client_sock;

  client_sock = accept(server->listen_sock, NULL, NULL);

  if (client_sock < 0) {
    g_log(NULL, G_LOG_LEVEL_CRITICAL, "accept() failed: errno = %d", errno);
    return FALSE;
  }

  g_log(NULL, G_LOG_LEVEL_DEBUG, "accepted client connection: fd = %d", client_sock);

  client = client_new(client_sock, server->uhuru);

  g_thread_pool_push(server->thread_pool, (gpointer)client, NULL);

  return TRUE;
}

#if 0
to be moved to linux main.c
  sock_path = uhuru_get_remote_url(server->uhuru);

#if 0
  if (unlink(sock_path) && errno != ENOENT) {
    perror("unlink");
    exit(EXIT_FAILURE);
  }
#endif
#endif

struct server *server_new(int server_sock)
{
  struct server *server;

  server = (struct server *)malloc(sizeof(struct server));
  assert(server != NULL);

  server->uhuru = uhuru_open();
  assert(server->uhuru != NULL);

  server->listen_sock = server_sock;

  server->channel = g_io_channel_unix_new(server->listen_sock);
  g_io_add_watch(server->channel, G_IO_IN, server_listen_cb, server);

  server->thread_pool = g_thread_pool_new(client_thread, server, -1, FALSE, NULL);

  return server;
}

void server_loop(struct server *server)
{
  GMainLoop *loop;

  loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
}
