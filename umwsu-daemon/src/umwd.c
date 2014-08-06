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

struct umwd {
  int inotify_fd;
  GHashTable *watch_table;
  struct umw *umw;
};

static void error(const char *msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

static void umwd_init(struct umwd *d)
{
  d->inotify_fd = inotify_init();
  if (d->inotify_fd == -1)
    error("inotify_init");

  d->watch_table = g_hash_table_new(g_direct_hash, g_direct_equal);
  d->umw = umw_open();
}

static void umwd_watch(struct umwd *d, const char *path, int recurse)
{
  int wd;

  fprintf(stderr, "adding watch on %s\n", path);

  if (recurse) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL)
      error("opendir");

    while((entry = readdir(dir)) != NULL) {
      char *entry_path;

      if (entry->d_type != DT_DIR || !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
	continue;
      
      if (asprintf(&entry_path, "%s/%s", path, entry->d_name) == -1)
	error("asprintf");

      umwd_watch(d, entry_path, 1);
      
      free(entry_path);
    }
  }

  wd = inotify_add_watch(d->inotify_fd, path, IN_CLOSE_WRITE | IN_CREATE);
  g_hash_table_insert(d->watch_table, GINT_TO_POINTER(wd), (gpointer)strdup(path));
}

static char *inotify_mask_str(uint32_t mask)
{
  static char buffer[256];

  buffer[0] = '\0';

#define M(_buff, _mask, _mask_bit) do { if ((_mask) & (_mask_bit)) strcat(_buff, #_mask_bit " "); } while(0)

  M(buffer, mask, IN_ACCESS);
  M(buffer, mask, IN_ATTRIB);
  M(buffer, mask, IN_CLOSE_WRITE);
  M(buffer, mask, IN_CLOSE_NOWRITE);
  M(buffer, mask, IN_CREATE);
  M(buffer, mask, IN_DELETE);
  M(buffer, mask, IN_DELETE_SELF);
  M(buffer, mask, IN_MODIFY);
  M(buffer, mask, IN_MOVE_SELF);
  M(buffer, mask, IN_MOVED_FROM);
  M(buffer, mask, IN_MOVED_TO);
  M(buffer, mask, IN_OPEN);

  return buffer;
}

static void umwd_process_event(struct umwd *d, struct inotify_event *event)
{
  char *full_path, *dir;

  dir = (char *)g_hash_table_lookup(d->watch_table, GINT_TO_POINTER(event->wd));

  if (dir == NULL)
    error("dir lookup");

  if (asprintf(&full_path, "%s/%s", dir, event->name) == -1)
    error("asprintf");

  fprintf(stderr, "%s: got inotify event %s\n", full_path, inotify_mask_str(event->mask));

  if (event->mask & IN_CLOSE_WRITE)
    umw_scan_file(d->umw, full_path);
}

void print_entry(gpointer key, gpointer value, gpointer user_data)
{
  fprintf(stderr, "hash table entry %d -> %s\n", GPOINTER_TO_INT(key), (char *)value);
}

static void umwd_loop(struct umwd *d)
{
  size_t event_size = sizeof(struct inotify_event) + NAME_MAX + 1;
  struct inotify_event *event;

#if 0
  g_hash_table_foreach(d->watch_table, print_entry, NULL);
#endif

  event = (struct inotify_event *)malloc(event_size);
  assert(event != NULL);

  while(1) {
    if (read(d->inotify_fd, event, event_size) < 0)
      error("read");

    umwd_process_event(d, event);
  }
}

static void usage(int argc, char **argv)
{
  fprintf(stderr, "usage: %s [-r] FILE|DIR ...\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
  struct umwd umwd;
  int argp = 1;
  int recurse = 0;
  
  if (argc < 2)
    usage(argc, argv);

  if (!strcmp(argv[argp], "-r")) {
    argp++;
    recurse = 1;
  }

  umwd_init(&umwd);

  while (argp < argc) {
    struct stat sb;

    if (stat(argv[argp], &sb) == -1) {
      perror("stat");
      exit(EXIT_FAILURE);
    }

    if (!S_ISDIR(sb.st_mode)) {
      fprintf(stderr, "%s: not a directory\n", argv[argp]);
      exit(EXIT_FAILURE);
    }

    umwd_watch(&umwd, argv[argp], recurse);

    argp++;
  }

  umwd_loop(&umwd);
}
