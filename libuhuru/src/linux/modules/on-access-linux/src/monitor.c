/*
  TODO:
  - implement fanotify non-perm event
*/

#define _GNU_SOURCE

#include <libuhuru/core.h>
#include "config/libuhuru-config.h"

#include "monitor.h"
#include "famonitor.h"
#include "imonitor.h"
#include "mount.h"
#include "onaccessmod.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

enum entry_flag {
  ENTRY_MOUNT = 1,
  ENTRY_DIR,
};

struct monitor_entry {
  const char *path;
  enum entry_flag flag;
};

struct access_monitor {
  int enable;
  int enable_permission;
  int enable_removable_media;
  GPtrArray *entries;

  int start_pipe[2];
  int command_pipe[2];

  GThread *monitor_thread;

  struct fanotify_monitor *fanotify_monitor;
  struct inotify_monitor *inotify_monitor;
  struct mount_monitor *mount_monitor;
};

static gboolean start_cb(GIOChannel *source, GIOCondition condition, gpointer data);
static gboolean command_cb(GIOChannel *source, GIOCondition condition, gpointer data);

static gpointer monitor_thread_fun(gpointer data);
static void scan_file_thread_fun(gpointer data, gpointer user_data);

static void entry_destroy_notify(gpointer data)
{
  struct monitor_entry *e = (struct monitor_entry *)data;

  free((void *)e->path);
  free(e);
}

struct access_monitor *access_monitor_new(struct uhuru *u)
{
  struct access_monitor *m = malloc(sizeof(struct access_monitor));
  GIOChannel *start_channel;

  m->enable = 0;
  m->enable_permission = 0;
  m->enable_removable_media = 0;

  m->entries = g_ptr_array_new_full(10, entry_destroy_notify);

  /* this pipe will be used to trigger creation of the monitor thread when entering main thread loop, */
  /* so that the monitor thread does not start before all modules are initialized  */
  /* and the daemon main loop is entered */
  if (pipe(m->start_pipe) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "pipe failed (%s)", strerror(errno));
    g_free(m);
    return NULL;
  }

  /* this pipe will be used to send commands to the monitor thread */
  if (pipe(m->command_pipe) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "pipe failed (%s)", strerror(errno));
    g_free(m);
    return NULL;
  }

  start_channel = g_io_channel_unix_new(m->start_pipe[0]);	
  g_io_add_watch(start_channel, G_IO_IN, start_cb, m);

  m->fanotify_monitor = fanotify_monitor_new(u);
  m->inotify_monitor = inotify_monitor_new(m);
  m->mount_monitor = NULL;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "init ok");

  return m;
}

int access_monitor_enable(struct access_monitor *m, int enable)
{
  m->enable = enable;

  return enable;
}

int access_monitor_enable_permission(struct access_monitor *m, int enable_permission)
{
  m->enable_permission = enable_permission;

  return enable_permission;
}

int access_monitor_enable_removable_media(struct access_monitor *m, int enable_removable_media)
{
  m->enable_removable_media = enable_removable_media;

  return enable_removable_media;
}

static void add_entry(struct access_monitor *m, const char *path, enum entry_flag flag)
{
  struct monitor_entry *e = malloc(sizeof(struct monitor_entry));

  e->path = strdup(path);
  e->flag = flag;

  g_ptr_array_add(m->entries, e);
}

static dev_t get_dev_id(const char *path)
{
  struct stat buf;

  if (stat(path, &buf) < 0)
    return -1;

  return buf.st_dev;
}

void access_monitor_add_mount(struct access_monitor *m, const char *mount_point)
{
  dev_t mount_dev_id, slash_dev_id;

  /* check that mount_point is not in the same partition as / */
  slash_dev_id = get_dev_id("/");
  if (slash_dev_id < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "cannot get device id for / (%s)", strerror(errno));
    return;
  }

  mount_dev_id = get_dev_id(mount_point);
  if (mount_dev_id < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "cannot get device id for %s (%s)", mount_point, strerror(errno));
    return;
  }

  if (mount_dev_id == slash_dev_id) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "\"%s\" is in same partition as \"/\"; adding \"/\" as monitored mount point is not supported", mount_point);
    return;
  }

  add_entry(m, mount_point, ENTRY_MOUNT);
}

void access_monitor_add_directory(struct access_monitor *m, const char *path)
{
  add_entry(m, path, ENTRY_DIR);
}

int access_monitor_start(struct access_monitor *m)
{
  char c = 'A';

  if (m == NULL)
    return 0;

  if (write(m->start_pipe[1], &c, 1) < 0)
    return -1;

  return 0;
}

static gboolean start_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct access_monitor *m = (struct access_monitor *)data;
  char c;

  if (read(m->start_pipe[0], &c, 1) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "read() in activation callback failed (%s)", strerror(errno));

    return FALSE;
  }

  if (c != 'A') {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "unexpected character ('%c' (0x%x) != 'A')", c, c);
    return FALSE;
  }

  /* commented out: closing the pipe leaded to an obscure race condition with other threads, resulting in a reuse */
  /* of the pipe input file descriptor (namely, for one associated with a client connection) and in IPC errors */
  /* g_io_channel_shutdown(source, FALSE, NULL); */

  m->monitor_thread = g_thread_new("access monitor thread", monitor_thread_fun, m);

  return TRUE;
}

static void mark_directory(struct access_monitor *m, const char *path)
{
  if (fanotify_monitor_mark_directory(m->fanotify_monitor, path, m->enable_permission) < 0)
    return;

  if (inotify_monitor_mark_directory(m->inotify_monitor, path) < 0)
    return;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "added mark for directory %s", path);
}

int access_monitor_unmark_directory(struct access_monitor *m, const char *path)
{
  if (fanotify_monitor_unmark_directory(m->fanotify_monitor, path, m->enable_permission) < 0)
    return -1;

  if (inotify_monitor_unmark_directory(m->inotify_monitor, path) < 0)
    return -1;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "removed mark for directory %s", path);

  return 0;
}

int access_monitor_recursive_mark_directory(struct access_monitor *m, const char *path)
{
  DIR *dir;
  struct dirent *entry;
  GString *entry_path;

  mark_directory(m, path);

  if ((dir = opendir(path)) == NULL) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "error opening directory %s (%s)", path, strerror(errno));
    return -1;
  }

  entry_path = g_string_new("");

  while((entry = readdir(dir)) != NULL) {
    if (entry->d_type != DT_DIR || !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;
      
    g_string_printf(entry_path, "%s/%s", path, entry->d_name);

    access_monitor_recursive_mark_directory(m, entry_path->str);
  }

  g_string_free(entry_path, TRUE);

  if (closedir(dir) < 0)
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "error closing directory %s (%s)", path, strerror(errno));

  return 0;
}

static void mark_mount_point(struct access_monitor *m, const char *path)
{
  if (fanotify_monitor_mark_mount(m->fanotify_monitor, path, m->enable_permission) < 0)
    return;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "added mark for mount point %s", path);
}

static void unmark_mount_point(struct access_monitor *m, const char *path)
{
  if (path == NULL)
    return;

  if (fanotify_monitor_unmark_mount(m->fanotify_monitor, path, m->enable_permission) < 0)
    return;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "removed mark for mount point %s", path);
}

static void mark_entries(struct access_monitor *m)
{
  int i;

  for(i = 0; i < m->entries->len; i++) {
    struct monitor_entry *e = (struct monitor_entry *)g_ptr_array_index(m->entries, i);

    if (e->flag == ENTRY_DIR)
      access_monitor_recursive_mark_directory(m, e->path);
    else
      mark_mount_point(m, e->path);
  }
}

static void mount_cb(enum mount_event_type ev_type, const char *path, void *user_data)
{
  struct access_monitor *m = (struct access_monitor *)user_data;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "received mount notification for %s (%s)", path, ev_type == EVENT_MOUNT ? "mount" : "umount");

  if (ev_type == EVENT_MOUNT)
    mark_mount_point(m, path);

  /* if ev_type is EVENT_UMOUNT, nothing to be done, the kernel has already removed the fanotify mark */
  /* and anyway, path is NULL, so... */
  /* unmark_mount_point(m, path); */
}

static gpointer monitor_thread_fun(gpointer data)
{
  struct access_monitor *m = (struct access_monitor *)data;
  GIOChannel *command_channel;
  GMainLoop *loop;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "started thread");

  if (fanotify_monitor_start(m->fanotify_monitor))
    return NULL;

  if (inotify_monitor_start(m->inotify_monitor))
    return NULL;

  /* if configured, add the mount monitor */
  if (m->enable_removable_media) {
    m->mount_monitor = mount_monitor_new(mount_cb, m);
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "added removable media monitor");
  }

  /* init all fanotify and inotify marks */
  mark_entries(m);

  command_channel = g_io_channel_unix_new(m->command_pipe[0]);	
  g_io_add_watch(command_channel, G_IO_IN, command_cb, m);

  loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(loop);
}

static gboolean command_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct access_monitor *m = (struct access_monitor *)data;
  char cmd;

  if (read(m->command_pipe[0], &cmd, 1) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "read() in command callback failed (%s)", strerror(errno));

    return FALSE;
  }

  switch(cmd) {
  case 'g':
    break;
  case 's':
    break;
  }

  return TRUE;
}

