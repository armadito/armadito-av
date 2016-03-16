#include <libuhuru/core.h>
#include <config/libuhuru-config.h>

#include "imonitor.h"
#include "monitor.h"
#include "onaccessmod.h"

#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

struct inotify_monitor {
  struct access_monitor *monitor;

  int inotify_fd;
  GHashTable *wd2path_table;
  GHashTable *path2wd_table;
};

static gboolean inotify_cb(GIOChannel *source, GIOCondition condition, gpointer data);

static void path_destroy_notify(gpointer data)
{
  free(data);
}

struct inotify_monitor *inotify_monitor_new(struct access_monitor *m)
{
  struct inotify_monitor *im = malloc(sizeof(struct inotify_monitor));

  im->monitor = m;

  im->wd2path_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, path_destroy_notify);
  im->path2wd_table = g_hash_table_new_full(g_str_hash, g_str_equal, path_destroy_notify, NULL);

  return im;
}

int inotify_monitor_start(struct inotify_monitor *im)
{
  GIOChannel *inotify_channel;
  GSource *source;

  im->inotify_fd = inotify_init();
  if (im->inotify_fd == -1) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "inotify_init failed (%s)", strerror(errno));
    return -1;
  }

  inotify_channel = g_io_channel_unix_new(im->inotify_fd);	

  /* g_io_add_watch(inotify_channel, G_IO_IN, inotify_cb, im); */
  source = g_io_create_watch(inotify_channel, G_IO_IN);
  g_source_set_callback(source, (GSourceFunc)inotify_cb, im, NULL);
  g_source_attach(source, access_monitor_get_main_context(im->monitor));
  g_source_unref(source);

  return 0;
}

int inotify_monitor_mark_directory(struct inotify_monitor *im, const char *path)
{
  int wd;

  wd = inotify_add_watch(im->inotify_fd, path, IN_ONLYDIR | IN_MOVE | IN_DELETE | IN_CREATE);
  if (wd == -1) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "adding inotify watch for %s failed (%s)", path, strerror(errno));
    return -1;
  }

  g_hash_table_insert(im->wd2path_table, GINT_TO_POINTER(wd), (gpointer)strdup(path));
  g_hash_table_insert(im->path2wd_table, (gpointer)strdup(path), GINT_TO_POINTER(wd));

  return 0;
}

int inotify_monitor_unmark_directory(struct inotify_monitor *im, const char *path)
{
  void *p;

  /* retrieve the watch descriptor associated to path */
  p = g_hash_table_lookup(im->path2wd_table, path);
  if (p == NULL) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "retrieving inotify watch id for %s failed", path);
  } else {
    int wd = GPOINTER_TO_INT(p);
  
    /* errors are ignored: if the watch descriptor is invalid, it means it is no longer being watched because of deletion */
    if (inotify_rm_watch(im->inotify_fd, wd) == -1) {
      uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "removing inotify watch %d for %s failed (%s)", wd, path, strerror(errno));
    }

    g_hash_table_remove(im->wd2path_table, GINT_TO_POINTER(wd));
  }

  g_hash_table_remove(im->path2wd_table, path);

  return 0;
}

#ifdef DEBUG
static void inotify_event_log(const struct inotify_event *e, const char *full_path)
{
  GString *s = g_string_new("");

  g_string_append_printf(s, "inotify event: wd=%2d ", e->wd);

  if (e->cookie > 0)
    g_string_append_printf(s, "cookie=%4d ", e->cookie);

  g_string_append_printf(s, "mask=");

#define M(_mask, _mask_bit) do { if ((_mask) & (_mask_bit)) g_string_append_printf(s, #_mask_bit " "); } while(0)

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
    g_string_append_printf(s, "name=%s", e->name);

  g_string_append_printf(s, " full_path=%s", full_path != NULL ? full_path : "null");

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "%s", s->str);

  g_string_free(s, TRUE);
}
#endif

static char *inotify_event_full_path(struct inotify_monitor *im, struct inotify_event *event)
{
  char *dir, *full_path;

  dir = (char *)g_hash_table_lookup(im->wd2path_table, GINT_TO_POINTER(event->wd));

  if (dir == NULL)
    return NULL;

  if (event->len) {
    GString *tmp = g_string_new("");

    g_string_printf(tmp, "%s/%s", dir, event->name);

    full_path = tmp->str;

    g_string_free(tmp, FALSE);
  } else {
    full_path = strdup(dir);
  }

  return full_path;
}

static void inotify_event_process(struct inotify_monitor *im, struct inotify_event *event)
{
  char *full_path;

  if (!(event->mask & IN_ISDIR))
    return;

  full_path = inotify_event_full_path(im, event);

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "inotify full path %s", full_path != NULL ? full_path : "(null)");

  if (full_path == NULL)
    return;

  if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO)
    access_monitor_recursive_mark_directory(im->monitor, full_path);
  else if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM)
    access_monitor_unmark_directory(im->monitor, full_path);

  free(full_path);
}

/* Size of buffer to use when reading inotify events */
#define INOTIFY_BUFFER_SIZE 8192

static gboolean inotify_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct inotify_monitor *im = (struct inotify_monitor *)data;
  char event_buffer[INOTIFY_BUFFER_SIZE];
  ssize_t len;

  assert(g_main_context_is_owner(access_monitor_get_main_context(im->monitor)));

  if ((len = read (im->inotify_fd, event_buffer, INOTIFY_BUFFER_SIZE)) > 0)  {
    char *p;

    p = event_buffer;
    while (p < event_buffer + len) {
      struct inotify_event *event = (struct inotify_event *) p;

      inotify_event_process(im, event);

      p += sizeof(struct inotify_event) + event->len;
    }
  }

  return TRUE;
}
