#include "server.h"
#include "client.h"
#include "lib/conf.h"
#include "lib/unixsock.h"

#include <libumwsu/scan.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>

struct server {
  int listen_sock;
  int epoll_fd;
  struct umwsu *umwsu;
};

struct poll_data {
  int fd;
  void *data;
};

static void server_add_polled_fd(struct server *server, int polled_fd, void *data);
static void server_remove_polled_fd(struct server *server, struct poll_data *p);

struct server *server_new(void)
{
  struct server *server;
  char *sock_path;

  server = (struct server *)malloc(sizeof(struct server));
  assert(server != NULL);

  server->umwsu = umwsu_open(0);
  assert(server->umwsu != NULL);

  umwsu_set_verbose(server->umwsu, 1);

  sock_path = conf_get(server->umwsu, "remote", "socket-path");
  assert(sock_path != NULL);

  server->epoll_fd = epoll_create(42);
  if (server->epoll_fd < 0) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }

  server->listen_sock = server_socket_create(sock_path);
  server_add_polled_fd(server, server->listen_sock, NULL);

  return server;
}

void server_loop(struct server *server)
{
#define MAX_EVENTS 10
  struct epoll_event events[MAX_EVENTS];

  while (1) {
    int n_ev, n;

    n_ev = epoll_wait(server->epoll_fd, events, MAX_EVENTS, -1);
    if (n_ev == -1) {
      perror("epoll_wait");
      exit(EXIT_FAILURE);
    }

    for (n = 0; n < n_ev; n++) {
      struct poll_data *p = (struct poll_data *)events[n].data.ptr;

      if (p->fd == server->listen_sock) {
	int conn_sock = server_socket_accept(server->listen_sock);

	fprintf(stderr, "accepted connection socket=%d\n", conn_sock);

	server_add_polled_fd(server, conn_sock, client_new(conn_sock, server->umwsu));

      } else {
	struct client *cl = (struct client *)p->data;

	if (client_process(cl) < 0)
	  server_remove_polled_fd(server, p);
      }
    }
  }
}

static void server_add_polled_fd(struct server *server, int polled_fd, void *data)
{
  struct epoll_event ev;
  struct poll_data *p = (struct poll_data *)malloc(sizeof(struct poll_data));

  ev.events = EPOLLIN;
  p->fd = polled_fd;
  p->data = data;
  ev.data.ptr = p;
  if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, p->fd, &ev) == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }
}

static void server_remove_polled_fd(struct server *server, struct poll_data *p)
{
  struct epoll_event ev;

  if (epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, p->fd, &ev) == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }

  /* should it be here??? */
  close(p->fd);

  free(p);
}

static int foo(int sock, void *data)
{
  char buff[100];

  memset(buff, 0, 100);
  read(sock, buff, 100);
  fprintf(stderr, "foo %d %s\n", sock, buff);
}

