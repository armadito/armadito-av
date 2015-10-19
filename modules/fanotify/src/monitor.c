/* 
   code inspired by: http://www.lanedo.com/filesystem-monitoring-linux-kernel/ 
*/

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
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <fcntl.h>
#include <linux/fanotify.h>
#include <sys/types.h>
#include <unistd.h>

struct access_monitor {
  int fanotify_fd;
  GPtrArray *paths;
  struct uhuru *uhuru;
  pid_t my_pid;
};

static gboolean access_monitor_cb(GIOChannel *source, GIOCondition condition, gpointer data);

void path_destroy_notify(gpointer data)
{
  free(data);
}

struct access_monitor *access_monitor_new(struct uhuru *u)
{
  struct access_monitor *m = g_new(struct access_monitor, 1);
  GIOChannel *channel;

  m->fanotify_fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_CONTENT, O_RDONLY | O_CLOEXEC | O_LARGEFILE | O_NOATIME);

  if (m->fanotify_fd < 0)
    g_log(NULL, G_LOG_LEVEL_ERROR, "fanotify: init failed (%s)", strerror(errno));

  m->paths = g_ptr_array_new_full(10, path_destroy_notify);

  channel = g_io_channel_unix_new(m->fanotify_fd);	

  g_io_add_watch(channel, G_IO_IN, access_monitor_cb, m);

  m->uhuru = u;

  m->my_pid = getpid();
  
  g_log(NULL, G_LOG_LEVEL_DEBUG, "fanotify: init ok");

  return m;
}

int access_monitor_add(struct access_monitor *m, const char *path)
{
  const char *tmp = strdup(path);

  if (fanotify_mark(m->fanotify_fd, FAN_MARK_ADD | FAN_MARK_MOUNT, FAN_OPEN_PERM, AT_FDCWD, tmp) < 0) {
    g_log(NULL, G_LOG_LEVEL_ERROR, "fanotify: adding %s failed (%s)", tmp, strerror(errno));

    return -1;
  }

  g_ptr_array_add(m->paths, (gpointer)tmp);

  g_log(NULL, G_LOG_LEVEL_DEBUG, "fanotify: added directory %s", tmp);

  return 0;
}

int access_monitor_remove(struct access_monitor *m, const char *path)
{
  if (fanotify_mark(m->fanotify_fd, FAN_MARK_REMOVE, FAN_OPEN_PERM, AT_FDCWD, path) < 0) {
    g_log(NULL, G_LOG_LEVEL_ERROR, "fanotify: removing %s failed (%s)", path, strerror(errno));

    return -1;
  }

  g_ptr_array_remove(m->paths, (gpointer)path);

  g_log(NULL, G_LOG_LEVEL_DEBUG, "fanotify: removed directory %s", path);

  return 0;
}

void access_monitor_free(struct access_monitor *m)
{
  int i;

  while (m->paths->len > 0) {
    const char *path = (const char *)g_ptr_array_index(m->paths, 0);

    access_monitor_remove(m, path);
  }

  g_ptr_array_free(m->paths, TRUE);

  close(m->fanotify_fd);

  free(m);

  g_log(NULL, G_LOG_LEVEL_DEBUG, "fanotify: free ok");
}


static char *get_file_path_from_fd(int fd, char *buffer, size_t buffer_size)
{
  ssize_t len;

  if (fd <= 0)
    return NULL;

  sprintf (buffer, "/proc/self/fd/%d", fd);
  if ((len = readlink (buffer, buffer, buffer_size - 1)) < 0)
    return NULL;

  buffer[len] = '\0';
  return buffer;
}

static __u32 file_status_2_response(enum uhuru_file_status status)
{
  switch(status) {
  case UHURU_SUSPICIOUS:
  case UHURU_MALWARE:
    return FAN_DENY;
  }

  return FAN_ALLOW;
}

static void event_process(struct access_monitor *m, struct fanotify_event_metadata *event)
{
  char file_path[PATH_MAX];
  char program_path[PATH_MAX];
  char *p;

  p = get_file_path_from_fd(event->fd, file_path, PATH_MAX);

  g_log(NULL, G_LOG_LEVEL_DEBUG, "fanotify: event 0x%llx fd %d path '%s'", event->mask, event->fd, p ? p : "unknown");

  if (event->mask & FAN_OPEN_PERM) {
    struct fanotify_response access;
    enum uhuru_file_status status;

    access.fd = event->fd;

    if (event->pid == m->my_pid) 
      access.response = FAN_ALLOW;
    else {
#if 1
      status = uhuru_scan_fd(m->uhuru, event->fd, p);
      access.response = file_status_2_response(status);
#endif
#if 0
      access.response = FAN_ALLOW;
#endif
    }

    write(m->fanotify_fd, &access, sizeof(access));

    g_log(NULL, G_LOG_LEVEL_DEBUG, "fanotify: fd %d path '%s' -> %s", 
	  event->fd, p ? p : "unknown", (access.response == FAN_ALLOW) ? "ALLOW" : "DENY");
  }

  close(event->fd);
}

/* Size of buffer to use when reading fanotify events */
/* 8192 is recommended by fanotify man page */
#define FANOTIFY_BUFFER_SIZE 8192

static gboolean access_monitor_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct access_monitor *m = (struct access_monitor *)data;
  char buf[FANOTIFY_BUFFER_SIZE];
  ssize_t len;

  if ((len = read (m->fanotify_fd, buf, FANOTIFY_BUFFER_SIZE)) > 0)  {
    struct fanotify_event_metadata *ev;

    for(ev = (struct fanotify_event_metadata *)buf; FAN_EVENT_OK(ev, len); ev = FAN_EVENT_NEXT(ev, len))
      event_process(m, ev);
  }

  return TRUE;
}

