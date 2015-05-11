#include "server.h"
#include "client.h"
#include "lib/conf.h"
#include <libumwsu/scan.h>

#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>
#include <glib-object.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct server {
  char *sock_path;
  struct umwsu *umwsu;
  GSocketService *service;
};

static gboolean server_incoming_cb(GSocketService *service, GSocketConnection *connection, GObject *source_object, gpointer user_data)
{
  GSocket *sock;
  GValue fd_value = G_VALUE_INIT;
  int fd;
  struct client *cl;
  struct server *server = (struct server *)user_data;

  sock = g_socket_connection_get_socket(connection);
  if (sock == NULL) {
    fprintf(stderr, "socket is NULL???\n");
    return TRUE;
  }

  g_value_init(&fd_value, G_TYPE_INT);
  g_object_get_property(G_OBJECT(sock), "fd", &fd_value);
  fd = g_value_get_int(&fd_value);

  cl = client_new(fd, server->umwsu);

  while (client_process(cl) >= 0)
    ;

  return FALSE;
}

static void server_get_sock_path(struct server *server)
{
  char *sock_dir;
  GString *sock_path;

  sock_dir = conf_get(server->umwsu, "remote", "socket-dir");
  assert(sock_dir != NULL);

  sock_path = g_string_new(sock_dir);
  g_string_append_printf(sock_path, "/uhuru-%s", getenv("USER"));
  server->sock_path = sock_path->str;
  g_string_free(sock_path, FALSE);
}

struct server *server_new(void)
{
  struct server *server;
  GError *error = NULL;
  GSocketAddress *sock_addr;

  server = (struct server *)malloc(sizeof(struct server));
  assert(server != NULL);

  server->umwsu = umwsu_open(0);
  assert(server->umwsu != NULL);
  umwsu_set_verbose(server->umwsu, 1);

  server_get_sock_path(server);

  if (unlink(server->sock_path) && errno != ENOENT) {
    perror("unlink");
    exit(EXIT_FAILURE);
  }

  sock_addr = g_unix_socket_address_new(server->sock_path);

  server->service = g_threaded_socket_service_new(-1);

  if (!g_socket_listener_add_address(G_SOCKET_LISTENER(server->service), sock_addr, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, NULL, NULL, &error)) {
    g_error("%s", error->message);
    exit(EXIT_FAILURE);
  }

  g_signal_connect(server->service, "incoming", G_CALLBACK(server_incoming_cb), server);
  g_socket_service_start(server->service);

  return server;
}

void server_loop(struct server *server)
{
  GMainLoop *loop;

  loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
}
