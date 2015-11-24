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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <fcntl.h>
#include <linux/fanotify.h>
#include <sys/types.h>
#include <unistd.h>

struct access_monitor {
  int enable_permission;
  int fanotify_fd;
  GPtrArray *paths;
  struct uhuru *uhuru;
  pid_t my_pid;
  GThreadPool *thread_pool;  
};

static gboolean access_monitor_cb(GIOChannel *source, GIOCondition condition, gpointer data);

void scan_file_thread_fun(gpointer data, gpointer user_data);

void path_destroy_notify(gpointer data)
{
  free(data);
}

struct access_monitor *access_monitor_new(struct uhuru *u)
{
  struct access_monitor *m = g_new(struct access_monitor, 1);
  GIOChannel *channel;

  m->enable_permission = 0;

  m->fanotify_fd = fanotify_init(FAN_CLASS_CONTENT | FAN_CLOEXEC, O_RDONLY | O_CLOEXEC | O_LARGEFILE | O_NOATIME);

  if (m->fanotify_fd < 0) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "fanotify: init failed (%s)", strerror(errno));
    g_free(m);
    return NULL;
  }

  m->paths = g_ptr_array_new_full(10, path_destroy_notify);

  channel = g_io_channel_unix_new(m->fanotify_fd);	

  g_io_add_watch(channel, G_IO_IN, access_monitor_cb, m);

  m->uhuru = u;

  m->my_pid = getpid();
  
  m->thread_pool = g_thread_pool_new(scan_file_thread_fun, m, -1, FALSE, NULL);

  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "fanotify: init ok");

  return m;
}

int access_monitor_enable_permission(struct access_monitor *m, int enable_permission)
{
  m->enable_permission = enable_permission;

  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "fanotify: %s", (m->enable_permission) ? "enabled" : "disabled");

  return m->enable_permission;
}

int access_monitor_add(struct access_monitor *m, const char *path)
{
  const char *tmp = strdup(path);

  /* move this to activation callback */
  if (fanotify_mark(m->fanotify_fd, FAN_MARK_ADD | FAN_MARK_MOUNT, FAN_OPEN_PERM, AT_FDCWD, tmp) < 0) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fanotify: adding %s failed (%s)", tmp, strerror(errno));

    return -1;
  }

  g_ptr_array_add(m->paths, (gpointer)tmp);

  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "fanotify: added directory %s", tmp);

  return 0;
}

int access_monitor_remove(struct access_monitor *m, const char *path)
{
  if (fanotify_mark(m->fanotify_fd, FAN_MARK_REMOVE, FAN_OPEN_PERM, AT_FDCWD, path) < 0) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "fanotify: removing %s failed (%s)", path, strerror(errno));

    return -1;
  }

  g_ptr_array_remove(m->paths, (gpointer)path);

  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "fanotify: removed directory %s", path);

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

  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "fanotify: free ok");
}


static char *get_file_path_from_fd(int fd, char *buffer, size_t buffer_size)
{
  ssize_t len;

  if (fd <= 0)
    return NULL;

  sprintf (buffer, "/proc/self/fd/%d", fd);
  if ((len = readlink(buffer, buffer, buffer_size - 1)) < 0)
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

static int write_response(struct access_monitor *m, int fd, const char *path, __u32 r)
{
  struct fanotify_response response;
  GLogLevelFlags log_level = G_LOG_LEVEL_INFO;
  const char *msg = "ALLOW";

  response.fd = fd;
  response.response = r;

  write(m->fanotify_fd, &response, sizeof(struct fanotify_response));
  
  if (r == FAN_DENY) {
    log_level = G_LOG_LEVEL_WARNING;
    msg = "DENY";
  }

  g_log(G_LOG_DOMAIN, log_level, "fanotify:  path '%s' -> %s", path ? path : "unknown", msg);

  close(fd);

  return r;
}

struct access_thread_data {
  int fd;
  const char *path;
};

void scan_file_thread_fun(gpointer data, gpointer user_data)
{
  enum uhuru_file_status status;
  struct access_monitor *m = (struct access_monitor *)user_data;
  struct access_thread_data *td = (struct access_thread_data *)data;
	
  status = uhuru_scan_simple(m->uhuru, td->path, NULL);

  write_response(m, td->fd, td->path, file_status_2_response(status));

  free((void *)td->path);
  free((void *)td);
}

static int perm_event_process(struct access_monitor *m, struct fanotify_event_metadata *event, const char *path)
{
  struct stat buf;
  struct access_thread_data *td;

  if (m->enable_permission == 0)  /* permission check is disabled, always allow */
    return write_response(m, event->fd, path, FAN_ALLOW);

  if (m->my_pid == event->pid)   /* file was opened by myself, always allow */
    return write_response(m, event->fd, path, FAN_ALLOW);

  if (fstat(event->fd, &buf) < 0)
    return write_response(m, event->fd, path, FAN_ALLOW);

  if (!S_ISREG(buf.st_mode))
    return write_response(m, event->fd, path, FAN_ALLOW);

  td = malloc(sizeof(struct access_thread_data));

  td->fd = event->fd;
  td->path = strdup(path);

  g_thread_pool_push(m->thread_pool, (gpointer)td, NULL);

  return 0;
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
    struct fanotify_event_metadata *event;

    for(event = (struct fanotify_event_metadata *)buf; FAN_EVENT_OK(event, len); event = FAN_EVENT_NEXT(event, len)) {
      char file_path[PATH_MAX + 1];
      char *p;

      p = get_file_path_from_fd(event->fd, file_path, PATH_MAX);

      if (event->mask & FAN_OPEN_PERM)
	perm_event_process(m, event, p);
      else
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "fanotify: unprocessed event 0x%llx fd %d path '%s'", event->mask, event->fd, p ? p : "unknown");
    }
  }

  return TRUE;
}

