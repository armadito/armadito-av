#include <libuhuru/core.h>

#include "pollset.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>

struct poll_set {
  int epoll_fd;
};

struct poll_set *poll_set_new(void)
{
  struct poll_set *s = malloc(sizeof(struct poll_set));

  s->epoll_fd = epoll_create(42);

  if (s->epoll_fd == -1) {
    uhuru_log(UHURU_LOG_SERVICE, UHURU_LOG_LEVEL_ERROR, "epoll_create failed (%s)", strerror(errno));
    free(s);
    return NULL;
  }

  return s;
}

struct poll_entry {
  poll_cb_t cb;
  void *user_data;
};

int poll_set_add_fd(struct poll_set *s, int fd, poll_cb_t cb, void *user_data)
{
  struct epoll_event ev;
  struct poll_entry *e = malloc(sizeof(struct poll_entry));

  ev.events = EPOLLIN;
  e->cb = cb;
  e->user_data = user_data;
  ev.data.ptr = e;

  if (epoll_ctl(s->epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    uhuru_log(UHURU_LOG_SERVICE, UHURU_LOG_LEVEL_ERROR, "epoll_ctl failed (%s)", strerror(errno));

    return -1;
  }

  return 0;
}

int poll_set_loop(struct poll_set *s)
{
#define MAX_EVENTS 1024
  while (1) {
    int n_events, n;
    struct epoll_event events[MAX_EVENTS];

    n_events = epoll_wait(s->epoll_fd, events, MAX_EVENTS, -1);

    if (n_events == -1) {
      uhuru_log(UHURU_LOG_SERVICE, UHURU_LOG_LEVEL_ERROR, "epoll_wait failed (%s)", strerror(errno));
      return -1;
    }

    for (n = 0; n < n_events; n++) {
      struct poll_entry *e = (struct poll_entry *)events[n].data.ptr;
      poll_cb_t cb = e->cb;

      (cb)(e->user_data);
    }
  }

  return 0;
}



/* ************************************************** */

#if 0
static void epoll_remove_stream(int epoll_fd, struct perseus_stream *stream)
{
  /* le descripteur de fichier supprimé pour epoll() est le descripteur retourné par la fonction perseus_stream_poll_fd() */
  if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, perseus_stream_poll_fd(stream), NULL) == -1) {
    perror("epoll_ctl: removing fd");
    exit(EXIT_FAILURE);
  }
}
#endif

