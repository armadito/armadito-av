/* 
   code inspired by: http://www.lanedo.com/filesystem-monitoring-linux-kernel/ 
*/

#define _GNU_SOURCE

#include <libuhuru/core.h>

#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <fcntl.h>
#include <linux/fanotify.h>

/* Size of buffer to use when reading fanotify events, 8192 recommended by fanotify man page */
#define FANOTIFY_BUFFER_SIZE 8192

struct fanotify_data {
  int fanotify_fd;
  /* a GArray and not a GPtrArray because GArray can be automatically NULL terminated */
    GArray *watched_dirs;
};

static enum uhuru_mod_status mod_fanotify_init(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = g_new(struct fanotify_data, 1);

  fa_data->watched_dirs = g_array_new(TRUE, TRUE, sizeof(const char *));
  module->data = fa_data;

  g_log(NULL, G_LOG_LEVEL_DEBUG, "fanotify init ok");

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_conf_set_watch_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  while (*argv != NULL) {
    const char *tmp = strdup(*argv);

    g_log(NULL, G_LOG_LEVEL_DEBUG, "got dir \"%s\"", *argv);

    g_array_append_val(fa_data->watched_dirs, tmp);

    argv++;
  }

  return UHURU_MOD_OK;
}

static char *get_file_path_from_fd(int fd, char *buffer, size_t buffer_size)
{
  ssize_t len;

  if (fd <= 0)
    return NULL;

  sprintf(buffer, "/proc/self/fd/%d", fd);
  if ((len = readlink(buffer, buffer, buffer_size - 1)) < 0)
    return NULL;

  buffer[len] = '\0';
  return buffer;
}

static void event_process(struct fanotify_data *fa_data, struct fanotify_event_metadata *event)
{
  char file_path[PATH_MAX];
  char program_path[PATH_MAX];
  char *p;

  p = get_file_path_from_fd(event->fd, file_path, PATH_MAX);
  g_log(NULL, G_LOG_LEVEL_DEBUG, "received event fd %d path '%s'", event->fd, p ? p : "unknown");

  if (event->mask & FAN_OPEN_PERM) {
    struct fanotify_response access;

    access.fd = event->fd;
    access.response = FAN_ALLOW;
    /* access.response = FAN_DENY; */

    write(fa_data->fanotify_fd, &access, sizeof(access));
  }

  close(event->fd);
}

static gboolean mod_fanotify_watch_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)data;
  char buf[FANOTIFY_BUFFER_SIZE];
  ssize_t len;

  g_log(NULL, G_LOG_LEVEL_DEBUG, "fanotify callback");

  if ((len = read(fa_data->fanotify_fd, buf, FANOTIFY_BUFFER_SIZE)) > 0)  {
    struct fanotify_event_metadata *ev;

    for(ev = (struct fanotify_event_metadata *)buf; FAN_EVENT_OK(ev, len); ev = FAN_EVENT_NEXT(ev, len))
      event_process(fa_data, ev);
  }

  return TRUE;
}

static enum uhuru_mod_status mod_fanotify_post_init(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;
  GIOChannel *channel;
  const char **dirs;

  if (fa_data->watched_dirs->len == 0) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "fanotify post_init : no directories to monitor!");
    return UHURU_MOD_OK;
  }

  fa_data->fanotify_fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_CONTENT, O_RDONLY | O_CLOEXEC | O_LARGEFILE | O_NOATIME);

  if (fa_data->fanotify_fd < 0) {
    g_log(NULL, G_LOG_LEVEL_ERROR, "fanotify post_init failed (%s)", strerror(errno));

    return UHURU_MOD_INIT_ERROR;
  }

  channel = g_io_channel_unix_new(fa_data->fanotify_fd);	
  g_io_add_watch(channel, G_IO_IN, mod_fanotify_watch_cb, fa_data);

  for(dirs = (const char **)fa_data->watched_dirs->data; *dirs != NULL; dirs++) {
    g_log(NULL, G_LOG_LEVEL_DEBUG, "fanotify marking %s", *dirs);

    if (fanotify_mark(fa_data->fanotify_fd, FAN_MARK_ADD | FAN_MARK_MOUNT, FAN_OPEN_PERM, AT_FDCWD, *dirs) < 0) {
      g_log(NULL, G_LOG_LEVEL_ERROR, "fanotify mark on %s failed (%s)", *dirs, strerror(errno));

      return UHURU_MOD_INIT_ERROR;
    }

    g_log(NULL, G_LOG_LEVEL_DEBUG, "fanotify marked %s", *dirs);
  }

  g_log(NULL, G_LOG_LEVEL_DEBUG, "fanotify post_init ok");

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_close(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

#if 0
static void shutdown_fanotify (int fanotify_fd)
{
  int i;

  for (i = 0; i < n_monitors; ++i)
    {
      /* Remove the mark, using same event mask as when creating it */
      fanotify_mark (fanotify_fd,
		     FAN_MARK_REMOVE,
		     event_mask,
		     AT_FDCWD,
		     monitors[i].path);
      free (monitors[i].path);
    }
  free (monitors);
  close (fanotify_fd);
}
#endif

  return UHURU_MOD_OK;
}

struct uhuru_conf_entry mod_fanotify_conf_table[] = {
  { "watch-dir", mod_fanotify_conf_set_watch_dir},
  { NULL, NULL},
};

struct uhuru_module module = {
  .init_fun = mod_fanotify_init,
  .conf_table = mod_fanotify_conf_table,
  .post_init_fun = mod_fanotify_post_init,
  .scan_fun = NULL,
  .close_fun = mod_fanotify_close,
  .info_fun = NULL,
  .name = "fanotify",
  .size = sizeof(struct fanotify_data),
};
