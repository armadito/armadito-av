#include "poll.h"
#include "unixsock.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>

struct poll_set {
  int listen_sock;
  int epoll_fd;
};

struct poll_data {
  int fd;
  void *data;
};

static void poll_set_add_fd(struct poll_set *s, int fd, void *data)
{
  struct epoll_event ev;
  struct poll_data *p = (struct poll_data *)malloc(sizeof(struct poll_data));

  ev.events = EPOLLIN;
  p->fd = fd;
  p->data = data;
  ev.data.ptr = p;
  if (epoll_ctl(s->epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }
}

static void poll_set_remove_fd(struct poll_set *s, struct poll_data *p)
{
  struct epoll_event ev;

  if (epoll_ctl(s->epoll_fd, EPOLL_CTL_DEL, p->fd, &ev) == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }

  close(p->fd);

  free(p);
}

struct poll_set *poll_set_new(int listen_sock)
{
  struct poll_set *s;

  s = (struct poll_set *)malloc(sizeof(struct poll_set));
  assert(s != NULL);

  s->listen_sock = listen_sock;

  s->epoll_fd = epoll_create(42);
  if (s->epoll_fd < 0) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }

  poll_set_add_fd(s, listen_sock, NULL);

  return s;
}

int poll_set_loop(struct poll_set *s)
{
#define MAX_EVENTS 10
  struct epoll_event events[MAX_EVENTS];

  while (1) {
    int n_ev, n;

    n_ev = epoll_wait(s->epoll_fd, events, MAX_EVENTS, -1);
    if (n_ev == -1) {
      perror("epoll_wait");
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "#event %d\n", n_ev);
    for (n = 0; n < n_ev; n++) {
      struct poll_data *p = (struct poll_data *)events[n].data.ptr;

      fprintf(stderr, "event %d\n", n);

      if (p->fd == s->listen_sock) {
	int conn_sock = server_socket_accept(s->listen_sock);

	poll_set_add_fd(s, conn_sock, NULL);
	fprintf(stderr, "accepted connection socket=%d\n", conn_sock);
      } else {
	char buff[100];
	int n_read;

	memset(buff, 0, 100);
	n_read = read(p->fd, buff, 100);
	if (n_read > 0)
	  fprintf(stderr, "foo %d %d %s\n", p->fd, n_read, buff);
	else {
	  poll_set_remove_fd(s, p);
	}
      }
    }
  }

  return 0;
}

void poll_set_close(struct poll_set *s)
{
}
