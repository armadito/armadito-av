#include "poll.h"
#include "unixsock.h"

#include <assert.h>
#include <stdlib.h>
#include <sys/epoll.h>

struct poll_set {
  int listen_sock;
  poll_cb_t cb;
  int epoll_fd;
};

typedef int (*poll_cb_t)(int sock, void *data);

static void poll_set_add_fd(struct poll_set *s, int fd, void *data)
{
  struct epoll_event ev;

  ev.events = EPOLLIN;
  ev.data.ptr = data;
  if (epoll_ctl(s->epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }
}

struct poll_set *poll_set_new(int listen_sock, poll_cb_t cb)
{
  struct poll_set *s;

  s = (struct poll_set *)malloc(sizeof(struct poll_set));
  assert(s != NULL);

  s->listen_sock = listen_sock;
  s->cb = cb;

  s->epoll_fd = epoll_create(42);
  if (s->epoll_fd < 0) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }

  poll_set_add_fd(s, listen_sock, NULL);

  return s;
}

int poll_set_loop(struct poll_set *s, void *data)
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

    for (n = 0; n < n_ev; n++) {
      if (events[n].data.fd == s->listen_sock) {
	int conn_sock = server_socket_accept(s->listen_sock);

	poll_set_add_fd(s, conn_sock, NULL);
      } else {
	(*s->cb)(events[n].data.fd, NULL);
      }
    }
  }

  return 0;
}

void poll_set_close(struct poll_set *s)
{
}
