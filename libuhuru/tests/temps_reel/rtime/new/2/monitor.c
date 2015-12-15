/* 
   code inspired by: http://www.lanedo.com/filesystem-monitoring-linux-kernel/ 
*/

#define _GNU_SOURCE

#include "monitor.h"
#include "mimetype.h"

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
  enum access_monitor_flags flags;
  int fanotify_fd;
  pid_t my_pid;
  GThreadPool *thread_pool;  
};

void scan_file_thread_fun(gpointer data, gpointer user_data);

void path_destroy_notify(gpointer data)
{
  free(data);
}

struct access_monitor *access_monitor_new(enum access_monitor_flags flags)
{
  struct access_monitor *m = malloc(sizeof(struct access_monitor));

  m->flags = flags;

  m->fanotify_fd = fanotify_init(FAN_CLASS_CONTENT | FAN_CLOEXEC, O_RDONLY | O_CLOEXEC | O_LARGEFILE | O_NOATIME);

  if (m->fanotify_fd < 0) {
    fprintf(stderr, "fanotify: fanotify_init failed (%s)\n", strerror(errno));
    g_free(m);
    return NULL;
  }

  m->my_pid = getpid();
  
  m->thread_pool = g_thread_pool_new(scan_file_thread_fun, m, 8, TRUE, NULL);

  return m;
}

int access_monitor_get_poll_fd(struct access_monitor *m)
{
  if (m == NULL)
    return -1;

  return m->fanotify_fd;
}

int access_monitor_add(struct access_monitor *m, const char *path, unsigned int flags)
{
  if (fanotify_mark(m->fanotify_fd, FAN_MARK_ADD | flags, FAN_OPEN_PERM, AT_FDCWD, path) < 0) {
    fprintf(stderr, "fanotify: activating %s failed (%s)\n", path, strerror(errno));

    return -1;
  }

  fprintf(stderr, "fanotify: added directory %s\n", path);

  return 0;
}

#if 0
int access_monitor_remove(struct access_monitor *m, const char *path)
{
  if (fanotify_mark(m->fanotify_fd, FAN_MARK_REMOVE, FAN_OPEN_PERM, AT_FDCWD, path) < 0) {
    fprintf(stderr, "fanotify: removing %s failed (%s)\n", path, strerror(errno));

    return -1;
  }

  fprintf(stderr, "fanotify: removed directory %s\n", path);

  return 0;
}
#endif

void access_monitor_free(struct access_monitor *m)
{
  close(m->fanotify_fd);

  free(m);

  fprintf(stderr, "fanotify: free ok");
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

static int write_response(struct access_monitor *m, int fd, __u32 r, const char *path, const char *reason)
{
  struct fanotify_response response;
  const char *auth = "ALLOW";

  response.fd = fd;
  response.response = r;

  write(m->fanotify_fd, &response, sizeof(struct fanotify_response));
  
  close(fd);

  if (m->flags & MONITOR_LOG_EVENT) {
    if (path == NULL)
      fprintf(stderr, "fanotify: fd %d %s (%s)\n", fd, auth, reason != NULL ? reason : "unknown");
    else
      fprintf(stderr, "fanotify: path %s %s (%s)\n", path, auth, reason != NULL ? reason : "unknown");
  }
    
  return r;
}

void scan_file_thread_fun(gpointer data, gpointer user_data)
{
}

static int perm_event_process(struct access_monitor *m, struct fanotify_event_metadata *event)
{
  char file_path[PATH_MAX + 1];
  char *p;
  struct stat buf;
  const char *mime_type;

  if (!(m->flags & MONITOR_ENABLE_PERM))  /* permission check is disabled, always allow */
    return write_response(m, event->fd, FAN_ALLOW, NULL, "permission is not activated");

  if (m->my_pid == event->pid)   /* file was opened by myself, always allow */
    return write_response(m, event->fd, FAN_ALLOW, NULL, "event PID is myself");

  if (fstat(event->fd, &buf) < 0)
    return write_response(m, event->fd, FAN_ALLOW, NULL, "stat failed");

  if (!S_ISREG(buf.st_mode))
    return write_response(m, event->fd, FAN_ALLOW, NULL, "fd is not a file");

  p = get_file_path_from_fd(event->fd, file_path, PATH_MAX);

  if (!(m->flags & MONITOR_TYPE_CHECK))
    return write_response(m, event->fd, FAN_ALLOW, p, "no check on MIME type");

  mime_type = mime_type_guess_fd(event->fd);

  write_response(m, event->fd, FAN_ALLOW, p, "checked MIME type");

  /* g_thread_pool_push(m->thread_pool, uhuru_file_context_clone(&file_context), NULL); */

  return 0;
}

/* Size of buffer to use when reading fanotify events */
/* 8192 is recommended by fanotify man page */
#define FANOTIFY_BUFFER_SIZE 8192

void access_monitor_cb(void *user_data)
{
  struct access_monitor *m = (struct access_monitor *)user_data;
  char buf[FANOTIFY_BUFFER_SIZE];
  ssize_t len;

  if ((len = read (m->fanotify_fd, buf, FANOTIFY_BUFFER_SIZE)) > 0)  {
    struct fanotify_event_metadata *event;

    for(event = (struct fanotify_event_metadata *)buf; FAN_EVENT_OK(event, len); event = FAN_EVENT_NEXT(event, len)) {
      if (event->mask & FAN_OPEN_PERM)
	perm_event_process(m, event);
      else
	fprintf(stderr, "fanotify: unprocessed event 0x%llx fd %d\n", event->mask, event->fd);
    }
  }
}

#if 0
int access_monitor_activate(struct access_monitor *m)
{
  int i;
  unsigned int mark_flags = FAN_MARK_ADD;

  if (m == NULL)
    return 0;

  if (m->flags & MONITOR_MOUNT)
    mark_flags |= FAN_MARK_MOUNT;

  for(i = 0; i < m->paths->len; i++) {
    const char *path = (const char *)g_ptr_array_index(m->paths, i);

    if (fanotify_mark(m->fanotify_fd, mark_flags, FAN_OPEN_PERM, AT_FDCWD, path) < 0) {
      fprintf(stderr, "fanotify: activating %s failed (%s)\n", path, strerror(errno));

      return -1;
    }

    fprintf(stderr, "fanotify: activated directory %s\n", path);
  }

  return 0;
}
#endif
