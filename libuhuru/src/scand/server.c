#include "server.h"
#include "client.h"
#include "lib/conf.h"
#include "lib/unixsock.h"
#include "lib/uhurup.h"
#include "lib/builtin-modules/remote.h"

#include <libuhuru/scan.h>

#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct server {
  int listen_sock;
  GIOChannel *channel;
  GThreadPool *thread_pool;  
  struct uhuru *uhuru;
};

static const char *server_get_sock_path(struct server *server)
{
  struct uhuru_module *remote_module;
  const char *sock_dir, *ret;
  GString *sock_path;

  remote_module = uhuru_get_module_by_name(server->uhuru, "remote");
  assert(remote_module != NULL);

  sock_dir = remote_module_get_sock_dir(remote_module);
  assert(sock_dir != NULL);
  /* was: argv = conf_get_value(server->uhuru, "remote", "socket-dir"); */

  sock_path = g_string_new(sock_dir);

  g_string_append_printf(sock_path, "/uhuru-%s", getenv("USER"));
  ret = sock_path->str;
  g_string_free(sock_path, FALSE);

  return ret;
}

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

  client_sock = server_socket_accept(server->listen_sock);

  client = client_new(client_sock, server->uhuru);

  g_thread_pool_push(server->thread_pool, (gpointer)client, NULL);

  return TRUE;
}

struct server *server_new(void)
{
  struct server *server;
  const char *sock_path;

  server = (struct server *)malloc(sizeof(struct server));
  assert(server != NULL);

  server->uhuru = uhuru_open(0);
  assert(server->uhuru != NULL);

  sock_path = server_get_sock_path(server);

  if (unlink(sock_path) && errno != ENOENT) {
    perror("unlink");
    exit(EXIT_FAILURE);
  }

  server->listen_sock = server_socket_create(sock_path);

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
