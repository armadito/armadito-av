#include <libuhuru/core.h>
#include "config/libuhuru-config.h"

#include "queue.h"
#include "stamp.h"
#include "onaccessmod.h"

#include <glib.h>
#include <linux/fanotify.h>
#include <stdlib.h>

struct watchdog {
  GThread *thread;
  struct queue *queue;
  int fanotify_fd;
};

#define N_FD 100
#define TIMEOUT  1000  /* micro seconds */
#define SLEEP    (TIMEOUT / 2)  /* micro seconds */

static gpointer watchdog_thread_fun(gpointer data)
{
  struct watchdog *w = (struct watchdog *)data;
  struct timespec timeout = { 0, TIMEOUT * ONE_MICROSECOND};
  struct timespec sleep_duration = { 0, SLEEP * ONE_MICROSECOND};
  struct timespec before;
  int i, n_fd;
  struct queue_entry entries[N_FD];

  while (1) {
    stamp_now(&before);
    stamp_sub(&before, &timeout);

    n_fd = queue_pop_timeout(w->queue, &before, entries, N_FD);

    if (n_fd) {
      uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, MODULE_NAME ": " "got %d fd in timeout", n_fd);

      for(i = 0; i < n_fd; i++)
	/* FIXME */
	write_response(w->fanotify_fd, entries[i].fd, FAN_ALLOW, entries[i].path, "timeout", 0);
    }

    nanosleep(&sleep_duration, NULL);
  }

  return NULL;
}

struct watchdog *watchdog_new(int fanotify_fd)
{
  struct watchdog *w = malloc(sizeof(struct watchdog));

  w->queue = queue_new();
  w->thread = g_thread_new("timeout thread", watchdog_thread_fun, w);
  w->fanotify_fd = fanotify_fd;

  return w;
}

void watchdog_add(struct watchdog *w, int fd, const char *path)
{
  struct timespec now;

  stamp_now(&now);

  queue_push(w->queue, fd, path, &now);
}

int watchdog_remove(struct watchdog *w, int fd, const char **p_path, struct timespec *after)
{
  int r;
  struct queue_entry entry;

  r = queue_pop_fd(w->queue, fd, &entry);

  if (r) {
    if (after != NULL) {
      stamp_now(after);
      stamp_sub(after, &entry.timestamp);
    }

    *p_path = entry.path;
  }

  return r;
}

