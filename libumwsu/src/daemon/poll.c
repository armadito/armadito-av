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

static void poll_set_add_fd(struct poll_set *s, int fd)
{
  struct epoll_event ev;

  ev.events = EPOLLIN;
  ev.data.fd = fd;
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

  poll_set_add_fd(s, listen_sock);

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
	poll_set_add_fd(s, server_socket_accept(s->listen_sock));
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

#if 0
           #define MAX_EVENTS 10
           struct epoll_event ev, events[MAX_EVENTS];
           int listen_sock, conn_sock, nfds, epollfd;

           /* Set up listening socket, 'listen_sock' (socket(),
              bind(), listen()) */

           epollfd = epoll_create(10);
           if (epollfd == -1) {
               perror("epoll_create");
               exit(EXIT_FAILURE);
           }

           ev.events = EPOLLIN;
           ev.data.fd = listen_sock;
           if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
               perror("epoll_ctl: listen_sock");
               exit(EXIT_FAILURE);
           }

           for (;;) {
               nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
               if (nfds == -1) {
                   perror("epoll_pwait");
                   exit(EXIT_FAILURE);
               }

               for (n = 0; n < nfds; ++n) {
                   if (events[n].data.fd == listen_sock) {
                       conn_sock = accept(listen_sock,
                                       (struct sockaddr *) &local, &addrlen);
                       if (conn_sock == -1) {
                           perror("accept");
                           exit(EXIT_FAILURE);
                       }
                       setnonblocking(conn_sock);
                       ev.events = EPOLLIN | EPOLLET;
                       ev.data.fd = conn_sock;
                       if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                                   &ev) == -1) {
                           perror("epoll_ctl: conn_sock");
                           exit(EXIT_FAILURE);
                       }
                   } else {
                       do_use_fd(events[n].data.fd);
                   }
               }
           }
#endif


