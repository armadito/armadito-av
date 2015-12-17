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
  GIOChannel *command_channel;

  int fanotify_fd;
  GIOChannel *notify_channel;

  GThread *monitor_thread;
};

static gboolean access_monitor_start_cb(GIOChannel *source, GIOCondition condition, gpointer data);
static gboolean access_monitor_command_cb(GIOChannel *source, GIOCondition condition, gpointer data);

static gpointer access_monitor_thread_fun(gpointer data);

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

int access_monitor_start(struct access_monitor *m)
{
  char c = 'A';

  if (m == NULL)
    return 0;

  if (write(m->start_pipe[1], &c, 1) < 0)
    return -1;

  return 0;
}

static gboolean access_monitor_start_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct access_monitor *m = (struct access_monitor *)data;
  char c;

  if (read(m->start_pipe[0], &c, 1) < 0 || c != 'A') {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "fanotify: read() in activation callback failed (%s)", strerror(errno));

    return FALSE;
  }

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "fanotify: started");

  g_io_channel_shutdown(source, FALSE, NULL);

  m->monitor_thread = g_thread_new("access monitor thread", access_monitor_thread_fun, m);

  return TRUE;
}

static gpointer access_monitor_thread_fun(gpointer data)
{
  struct access_monitor *m = (struct access_monitor *)data;
  GMainLoop *loop;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "fanotify: started thread");

  m->command_channel = g_io_channel_unix_new(m->command_pipe[0]);	
  g_io_add_watch(m->command_channel, G_IO_IN, access_monitor_command_cb, m);

  /* add the fanotify file desc to the thread loop */

  /* if configured, add the mount monitor */

  /* init all fanotify mark */

  loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(loop);
}

static gboolean access_monitor_command_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct access_monitor *m = (struct access_monitor *)data;
  char cmd;

  if (read(m->command_pipe[0], &cmd, 1) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "fanotify: read() in command callback failed (%s)", strerror(errno));

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

