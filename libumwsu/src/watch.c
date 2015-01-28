#include <libumwsu/scan.h>

#include <assert.h>
#include <glib.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <dirent.h>

struct umwsu_watch {
  int inotify_fd;
  GHashTable *watch_table;
  char *event_buffer;
  ssize_t nread;
  char *current_event;
};

static void error(const char *msg)
{
  perror(msg);
  /* exit(EXIT_FAILURE); */
}

#define N_EVENTS 10
#define EVENT_BUFFER_LEN (N_EVENTS * (sizeof(struct inotify_event) + NAME_MAX + 1))

struct umwsu_watch *umwsu_watch_new(void)
{
  struct umwsu_watch *w;

  w = (struct umwsu_watch *)malloc(sizeof(struct umwsu_watch));
  assert(w != 0);

  w->inotify_fd = inotify_init();
  if (w->inotify_fd == -1)
    error("inotify_init");

  w->watch_table = g_hash_table_new(g_direct_hash, g_direct_equal);

  w->event_buffer = (char *)malloc(EVENT_BUFFER_LEN);
  assert(w->event_buffer != NULL);

  w->nread = 0;
  w->current_event = w->event_buffer;

  return w;
}

static void umwsu_watch_add_aux(struct umwsu_watch *w, const char *path, int recurse)
{
  int wd;

  if (recurse) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL) {
      error("opendir");
      return;
    }

    while((entry = readdir(dir)) != NULL) {
      char *entry_path;

      if (entry->d_type != DT_DIR || !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
	continue;
      
      if (asprintf(&entry_path, "%s/%s", path, entry->d_name) == -1)
	error("asprintf");

      umwsu_watch_add_aux(w, entry_path, 1);
      
      free(entry_path);
    }
  }

  wd = inotify_add_watch(w->inotify_fd, path, IN_ALL_EVENTS);

  if (wd == -1) {
    error("inotify_add_watch");
    return;
  }

  fprintf(stderr, "adding watch %d on %s\n", wd, path);

  g_hash_table_insert(w->watch_table, GINT_TO_POINTER(wd), (gpointer)strdup(path));
}

static void inotify_event_print(FILE *out, const struct inotify_event *e, char *full_path)
{
  fprintf(out, "umwsu_daemon: inotify event: wd = %2d ", e->wd);

  if (e->cookie > 0)
    fprintf(out, "cookie = %4d ", e->cookie);

  fprintf(out, "mask = ");

#define M(_mask, _mask_bit) do { if ((_mask) & (_mask_bit)) fprintf(out, #_mask_bit " "); } while(0)

  M(e->mask, IN_ACCESS);
  M(e->mask, IN_ATTRIB);
  M(e->mask, IN_CLOSE_NOWRITE);
  M(e->mask, IN_CLOSE_WRITE);
  M(e->mask, IN_CREATE);
  M(e->mask, IN_DELETE);
  M(e->mask, IN_DELETE_SELF);
  M(e->mask, IN_IGNORED);
  M(e->mask, IN_ISDIR);
  M(e->mask, IN_MODIFY);
  M(e->mask, IN_MOVE_SELF);
  M(e->mask, IN_MOVED_FROM);
  M(e->mask, IN_MOVED_TO);
  M(e->mask, IN_OPEN);
  M(e->mask, IN_Q_OVERFLOW);
  M(e->mask, IN_UNMOUNT);

  if (e->len > 0)
    fprintf(out, " name = %s", e->name);

  fprintf(out, " full_path = %s\n", full_path);
}

static char *umwsu_watch_event_full_path(struct umwsu_watch *w, struct inotify_event *event)
{
  char *full_path, *dir;

  dir = (char *)g_hash_table_lookup(w->watch_table, GINT_TO_POINTER(event->wd));

  if (dir == NULL)
    error("dir lookup");

  if (asprintf(&full_path, "%s/%s", dir, event->name) == -1)
    error("asprintf");

  return full_path;
}

static enum umwsu_watch_event_type umwsu_watch_event_type(struct inotify_event *iev)
{
  if (iev->mask & IN_ISDIR) {
    if (iev->mask & IN_CREATE)
      return UMWSU_WATCH_DIRECTORY_CREATE;
    else if (iev->mask & IN_OPEN)
      return UMWSU_WATCH_DIRECTORY_OPEN;
    else if (iev->mask & IN_CLOSE_NOWRITE)
      return UMWSU_WATCH_DIRECTORY_CLOSE_NO_WRITE;
    else if (iev->mask & IN_CLOSE_WRITE)
      return UMWSU_WATCH_DIRECTORY_CLOSE_WRITE;
    else if (iev->mask & IN_DELETE)
      return UMWSU_WATCH_DIRECTORY_DELETE;
  } else {
    if (iev->mask & IN_CREATE)
      return UMWSU_WATCH_FILE_CREATE;
    else if (iev->mask & IN_OPEN)
      return UMWSU_WATCH_FILE_OPEN;
    else if (iev->mask & IN_CLOSE_NOWRITE)
      return UMWSU_WATCH_FILE_CLOSE_NO_WRITE;
    else if (iev->mask & IN_CLOSE_WRITE)
      return UMWSU_WATCH_FILE_CLOSE_WRITE;
    else if (iev->mask & IN_DELETE)
      return UMWSU_WATCH_FILE_DELETE;
  }

  return UMWSU_WATCH_NONE;
}

void umwsu_watch_add(struct umwsu_watch *w, const char *dir)
{
  umwsu_watch_add_aux(w, dir, 1);
}

static void umwsu_watch_process_event(struct umwsu_watch *w, struct inotify_event *iev, char *full_path)
{
  unsigned int delay = 200000;

  if (!(iev->mask & IN_ISDIR))
    return;

  usleep(delay);

#if 0
  if (iev->mask & IN_CREATE)
    umwsu_watch_add_aux(w, full_path, 0);
#endif
}

int umwsu_watch_wait(struct umwsu_watch *w, struct umwsu_watch_event *event)
{
  struct inotify_event *iev;

  if (w->current_event >= w->event_buffer + w->nread) {
    w->nread = read(w->inotify_fd, w->event_buffer, EVENT_BUFFER_LEN);
    
    if (w->nread == 0)
      error("read returned 0");

    if (w->nread < 0)
      error("read");

#if 0
    fprintf(stderr, "read %ld from %d\n", w->nread, w->inotify_fd);
#endif

    w->current_event = w->event_buffer;
  }

  iev = (struct inotify_event *)w->current_event;

  if (event->full_path != NULL)
    free(event->full_path);

  event->full_path = umwsu_watch_event_full_path(w, iev);
  event->event_type = umwsu_watch_event_type(iev);

  inotify_event_print(stderr, iev, event->full_path);

  umwsu_watch_process_event(w, iev, event->full_path);

  w->current_event += sizeof(struct inotify_event) + iev->len;

  return 0;
}

