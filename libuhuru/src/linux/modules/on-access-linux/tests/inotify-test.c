/* compile with: */
/* gcc $(pkg-config --cflags glib-2.0) inotify-test.c -o inotify-test $(pkg-config --libs glib-2.0) */

/* #include <libuhuru/scan.h> */

#include <assert.h>
#include <glib.h>

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <dirent.h>

struct uhuru_daemon {
  int inotify_fd;
  GHashTable *watch_table;
};

static void error(const char *msg, int do_exit, const char *x_msg)
{
  fprintf(stderr, "%s: %s", msg, strerror(errno));
  if (x_msg != NULL)
    fprintf(stderr, " (%s)", x_msg);
  fprintf(stderr, "\n");
  
  if (do_exit)
    exit(EXIT_FAILURE);
}

static void uhuru_daemon_init(struct uhuru_daemon *d)
{
  d->inotify_fd = inotify_init();
  if (d->inotify_fd == -1)
    error("inotify_init", 1, NULL);

  d->watch_table = g_hash_table_new(g_direct_hash, g_direct_equal);
}

static void uhuru_daemon_watch(struct uhuru_daemon *d, const char *path)
{
  int wd;
  uint32_t in_mask = IN_ONLYDIR | IN_MOVE | IN_DELETE | IN_CREATE;
  DIR *dir;
  struct dirent *entry;

  wd = inotify_add_watch(d->inotify_fd, path, in_mask);

  if (wd == -1)
    error("inotify_add_watch", 0, path);

  printf("adding watch %d on %s\n", wd, path);

  g_hash_table_insert(d->watch_table, GINT_TO_POINTER(wd), (gpointer)strdup(path));

  dir = opendir(path);
  if (dir == NULL) {
    error("opendir", 0, path);
    return;
  }

  while((entry = readdir(dir)) != NULL) {
    char *entry_path;

    if (entry->d_type != DT_DIR || !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;
      
    if (asprintf(&entry_path, "%s/%s", path, entry->d_name) == -1)
      error("asprintf", 0, NULL);

    uhuru_daemon_watch(d, entry_path);
      
    free(entry_path);
  }

  if (closedir(dir) < 0)
    error("closedir", 0, path);
}

static void inotify_event_print(FILE *out, const struct inotify_event *e)
{
  fprintf(out, "inotify event: wd = %2d ", e->wd);

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
    fprintf(out, "name = %s", e->name);

  fprintf(out, "\n");
}

static char *uhuru_daemon_event_full_path(struct uhuru_daemon *d, struct inotify_event *event)
{
  char *full_path, *dir;

  dir = (char *)g_hash_table_lookup(d->watch_table, GINT_TO_POINTER(event->wd));

  if (dir == NULL)
    error("dir lookup", 1, NULL);

  if (event->len) {
    if (asprintf(&full_path, "%s/%s", dir, event->name) == -1)
      error("asprintf", 0, NULL);
  } else {
    full_path = strdup(dir);
  }

  return full_path;
}

static void uhuru_daemon_process_dir_event(struct uhuru_daemon *d, struct inotify_event *event, const char *what)
{
  char *full_path = uhuru_daemon_event_full_path(d, event);

  fprintf(stderr, "processing dir event %s path = %s\n", what, full_path);

  free(full_path);
}

static void uhuru_daemon_process_event(struct uhuru_daemon *d, struct inotify_event *event)
{
  if (!(event->mask & IN_ISDIR))
    return;

  inotify_event_print(stderr, event);

  if (event->mask & IN_CREATE && event->mask & IN_ISDIR)
    uhuru_daemon_process_dir_event(d, event, "create");
  if (event->mask & IN_DELETE && event->mask & IN_ISDIR)
    uhuru_daemon_process_dir_event(d, event, "delete");
  if (event->mask & IN_MOVE_SELF && event->mask & IN_ISDIR)
    uhuru_daemon_process_dir_event(d, event, "move self");
  if (event->mask & IN_MOVED_FROM && event->mask & IN_ISDIR)
    uhuru_daemon_process_dir_event(d, event, "moved from");
  if (event->mask & IN_MOVED_TO && event->mask & IN_ISDIR)
    uhuru_daemon_process_dir_event(d, event, "move to");
}

/* Size of buffer to use when reading inotify events */
#define INOTIFY_BUFFER_SIZE 8192

static gboolean inotify_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct uhuru_daemon *d = (struct uhuru_daemon *)data;
  char event_buffer[INOTIFY_BUFFER_SIZE];
  ssize_t len;

  if ((len = read (d->inotify_fd, event_buffer, INOTIFY_BUFFER_SIZE)) > 0)  {
    char *p;

    p = event_buffer;
    while (p < event_buffer + len) {
      struct inotify_event *event = (struct inotify_event *) p;

      uhuru_daemon_process_event(d, event);

      p += sizeof(struct inotify_event) + event->len;
    }
  }

  return TRUE;
}

static void uhuru_daemon_loop(struct uhuru_daemon *d)
{
  GIOChannel *inotify_channel;
  GMainLoop *loop;

  inotify_channel = g_io_channel_unix_new(d->inotify_fd);
  g_io_add_watch(inotify_channel, G_IO_IN, inotify_cb, d);

  loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(loop);
}

static void usage(int argc, char **argv)
{
  fprintf(stderr, "usage: %s DIR ...\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
  struct uhuru_daemon uhuru_daemon;
  int argp = 1;
  
  if (argc < 2)
    usage(argc, argv);

  uhuru_daemon_init(&uhuru_daemon);

  while (argp < argc) {
    struct stat sb;

    if (stat(argv[argp], &sb) == -1)
      error("stat", 1, argv[argp]);

    if (!S_ISDIR(sb.st_mode)) {
      fprintf(stderr, "%s: not a directory\n", argv[argp]);
      exit(EXIT_FAILURE);
    }

    uhuru_daemon_watch(&uhuru_daemon, argv[argp]);

    argp++;
  }

  uhuru_daemon_loop(&uhuru_daemon);
}
