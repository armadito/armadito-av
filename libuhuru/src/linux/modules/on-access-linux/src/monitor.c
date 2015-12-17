#define _GNU_SOURCE

#include <libuhuru/core.h>

#include "monitor.h"

#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <fcntl.h>
#include <linux/fanotify.h>
#include <sys/types.h>
#include <unistd.h>

struct access_monitor {
  struct uhuru *uhuru;
  struct uhuru_scan_conf *scan_conf;

  int enable;
  int enable_permission;
  int enable_removable_media;
  GPtrArray *entries;

  pid_t my_pid;

  int start_pipe[2];
  int command_pipe[2];

  int fanotify_fd;
  GIOChannel *notify_channel;
};

static gboolean access_monitor_start_cb(GIOChannel *source, GIOCondition condition, gpointer data);
static gboolean access_monitor_command_cb(GIOChannel *source, GIOCondition condition, gpointer data);

void access_monitor_thread_fun(gpointer data, gpointer user_data);

static void path_destroy_notify(gpointer data)
{
  free(data);
}

static void entry_destroy_notify(gpointer data)
{
  free(data);
}

struct access_monitor *access_monitor_new(struct uhuru *u)
{
  struct access_monitor *m = g_new(struct access_monitor, 1);
  GIOChannel *start_channel;

  m->uhuru = u;
  m->scan_conf = uhuru_scan_conf_on_access();

  m->enable = 0;
  m->enable_permission = 0;
  m->enable_removable_media = 0;

  m->entries = g_ptr_array_new_full(10, entry_destroy_notify);

  m->my_pid = getpid();
  
  /* this pipe will be used to trigger creation of the monitor thread when entering main thread loop, */
  /* so that the monitor thread does not start before all modules are initialized and the daemon main loop is entered */
  if (pipe(m->start_pipe) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "fanotify: pipe failed (%s)", strerror(errno));
    g_free(m);
    return NULL;
  }

  /* this pipe will be used to send commands to the monitor thread */
  if (pipe(m->command_pipe) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "fanotify: pipe failed (%s)", strerror(errno));
    g_free(m);
    return NULL;
  }

  start_channel = g_io_channel_unix_new(m->start_pipe[0]);	
  g_io_add_watch(start_channel, G_IO_IN, access_monitor_start_cb, m);

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "fanotify: init ok");

  return m;
}

static gboolean access_monitor_start_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  return TRUE;
}
