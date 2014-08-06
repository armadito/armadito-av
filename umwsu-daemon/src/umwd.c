#include <libumwsu/scan.h>

#define _GNU_SOURCE
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
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
  g_hash_table_insert(d->watch_table, GINT_TO_POINTER(wd), (gpointer)path);
}

static void umwd_loop(struct umwd *p)
{
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
