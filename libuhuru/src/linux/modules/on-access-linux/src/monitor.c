#define _GNU_SOURCE

#include <libuhuru/core.h>

#include "config/libuhuru-config.h"

#include "monitor.h"
#include "mount.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <limits.h>
#include <linux/fanotify.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/signalfd.h>
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

  int inotify_fd;
  GHashTable *wd2path_table;
  GHashTable *path2wd_table;

  GThread *monitor_thread;
  GThreadPool *thread_pool;  
};

static gboolean access_monitor_start_cb(GIOChannel *source, GIOCondition condition, gpointer data);
static gboolean access_monitor_command_cb(GIOChannel *source, GIOCondition condition, gpointer data);
static gboolean access_monitor_fanotify_cb(GIOChannel *source, GIOCondition condition, gpointer data);
static gboolean access_monitor_inotify_cb(GIOChannel *source, GIOCondition condition, gpointer data);

static gpointer access_monitor_thread_fun(gpointer data);
static void scan_file_thread_fun(gpointer data, gpointer user_data);

static void path_destroy_notify(gpointer data)
{
  free(data);
}

static void entry_destroy_notify(gpointer data)
{
  struct monitor_entry *e = (struct monitor_entry *)data;

  free((void *)e->path);
  free(e);
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
  /* so that the monitor thread does not start before all modules are initialized  */
  /* and the daemon main loop is entered */
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

  m->wd2path_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, path_destroy_notify);
  m->path2wd_table = g_hash_table_new_full(g_str_hash, g_str_equal, path_destroy_notify, NULL);

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "fanotify: init ok");

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

static void access_monitor_add_entry(struct access_monitor *m, const char *path, enum entry_flag flag)
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
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "fanotify: cannot get device id for / (%s)", strerror(errno));
    return;
  }

  mount_dev_id = get_dev_id(mount_point);
  if (mount_dev_id < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "fanotify: cannot get device id for %s (%s)", mount_point, strerror(errno));
    return;
  }

  if (mount_dev_id == slash_dev_id) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "fanotify: \"%s\" is in same partition as \"/\"; adding \"/\" as monitored mount point is not supported", mount_point);
    return;
  }

  access_monitor_add_entry(m, mount_point, ENTRY_MOUNT);
}

void access_monitor_add_directory(struct access_monitor *m, const char *path)
{
  access_monitor_add_entry(m, path, ENTRY_DIR);
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

  if (read(m->start_pipe[0], &c, 1) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "fanotify: read() in activation callback failed (%s)", strerror(errno));

    return FALSE;
  }

  if (c != 'A') {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "fanotify: unexpected character ('%c' (0x%x) != 'A')", c, c);
    return FALSE;
  }

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "fanotify: started");

  /* commented out: closing the pipe leaded to an obscure race condition with other threads, resulting in a reuse */
  /* of the pipe input file descriptor (namely, for one associated with a client connection) and in IPC errors */
  /* g_io_channel_shutdown(source, FALSE, NULL); */

  m->monitor_thread = g_thread_new("access monitor thread", access_monitor_thread_fun, m);

  return TRUE;
}

static void access_monitor_mark_directory(struct access_monitor *m, const char *path)
{
  int wd;
  DIR *dir;
  struct dirent *entry;
  GString *entry_path;
  uint64_t fan_mask;

  wd = inotify_add_watch(m->inotify_fd, path, IN_ONLYDIR | IN_MOVE | IN_DELETE | IN_CREATE);
  if (wd == -1) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "fanotify: watching %s failed (%s)", path, strerror(errno));
    return;
  }

  g_hash_table_insert(m->wd2path_table, GINT_TO_POINTER(wd), (gpointer)strdup(path));
  g_hash_table_insert(m->path2wd_table, (gpointer)strdup(path), GINT_TO_POINTER(wd));

  fan_mask = (m->enable_permission ? FAN_OPEN_PERM : FAN_CLOSE_WRITE) | FAN_EVENT_ON_CHILD;
  if (fanotify_mark(m->fanotify_fd, FAN_MARK_ADD, fan_mask, AT_FDCWD, path) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "fanotify: marking %s failed (%s)", path, strerror(errno));
    return;
  } else
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "fanotify: marked directory %s", path);

  if ((dir = opendir(path)) == NULL) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "fanotify: error opening directory %s (%s)", path, strerror(errno));
    return;
  }

  entry_path = g_string_new("");

  while((entry = readdir(dir)) != NULL) {
    if (entry->d_type != DT_DIR || !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;
      
    g_string_printf(entry_path, "%s/%s", path, entry->d_name);

    access_monitor_mark_directory(m, entry_path->str);
  }

  g_string_free(entry_path, TRUE);

  if (closedir(dir) < 0)
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "fanotify: error closing directory %s (%s)", path, strerror(errno));
}

static void access_monitor_mark_mount_point(struct access_monitor *m, const char *path)
{
  uint64_t fan_mask = m->enable_permission ? FAN_OPEN_PERM : FAN_CLOSE_WRITE;

  if (fanotify_mark(m->fanotify_fd, FAN_MARK_ADD | FAN_MARK_MOUNT, fan_mask, AT_FDCWD, path) < 0)
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "fanotify: marking %s failed (%s)", path, strerror(errno));
  else
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "fanotify: marked mount point %s", path);
}

static void access_monitor_mark_entries(struct access_monitor *m)
{
  int i;

  for(i = 0; i < m->entries->len; i++) {
    struct monitor_entry *e = (struct monitor_entry *)g_ptr_array_index(m->entries, i);

    if (e->flag == ENTRY_DIR)
      access_monitor_mark_directory(m, e->path);
    else
      access_monitor_mark_mount_point(m, e->path);
  }
}

static gpointer access_monitor_thread_fun(gpointer data)
{
  struct access_monitor *m = (struct access_monitor *)data;
  GMainLoop *loop;
  GIOChannel *fanotify_channel, *inotify_channel;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "fanotify: started thread");

  m->command_channel = g_io_channel_unix_new(m->command_pipe[0]);	
  g_io_add_watch(m->command_channel, G_IO_IN, access_monitor_command_cb, m);

  m->thread_pool = g_thread_pool_new(scan_file_thread_fun, m, -1, FALSE, NULL);

  /* add the fanotify file desc to the thread loop */
  m->fanotify_fd = fanotify_init(FAN_CLASS_CONTENT | FAN_UNLIMITED_QUEUE | FAN_UNLIMITED_MARKS, O_LARGEFILE | O_RDONLY);
  if (m->fanotify_fd < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "fanotify: fanotify_init failed (%s)", strerror(errno));
    return NULL;
  }

  fanotify_channel = g_io_channel_unix_new(m->fanotify_fd);	
  g_io_add_watch(fanotify_channel, G_IO_IN, access_monitor_fanotify_cb, m);

  m->inotify_fd = inotify_init();
  if (m->inotify_fd == -1) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "fanotify: inotify_init failed (%s)", strerror(errno));
    return NULL;
  }

  inotify_channel = g_io_channel_unix_new(m->inotify_fd);	
  g_io_add_watch(inotify_channel, G_IO_IN, access_monitor_inotify_cb, m);

  /* if configured, add the mount monitor */

  /* init all fanotify mark */
  access_monitor_mark_entries(m);

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

static int write_response(struct access_monitor *m, int fd, __u32 r, const char *path, const char *reason)
{
  struct fanotify_response response;
  GLogLevelFlags log_level = UHURU_LOG_LEVEL_INFO;
  const char *auth = "ALLOW";

  response.fd = fd;
  response.response = r;

  write(m->fanotify_fd, &response, sizeof(struct fanotify_response));
  
  close(fd);

  if (r == FAN_DENY) {
    log_level = UHURU_LOG_LEVEL_WARNING;
    auth = "DENY";
  }

  if (path == NULL)
    fprintf(stderr, "fanotify: fd %d %s (%s)\n", fd, auth, reason != NULL ? reason : "unknown");
  else
    fprintf(stderr, "fanotify: path %s %s (%s)\n", path, auth, reason != NULL ? reason : "unknown");

  return r;
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

void scan_file_thread_fun(gpointer data, gpointer user_data)
{
  struct uhuru_file_context *file_context = (struct uhuru_file_context *)data;
  struct access_monitor *m = (struct access_monitor *)user_data;
  struct uhuru_scan *scan = uhuru_scan_new(m->uhuru, -1);
  enum uhuru_file_status status;
	
  status = uhuru_scan_context(scan, file_context);

  write_response(m, file_context->fd, file_status_2_response(status), file_context->path, "file was scanned");

  uhuru_file_context_free(file_context);

  uhuru_scan_free(scan);
}

static int perm_event_process(struct access_monitor *m, struct fanotify_event_metadata *event)
{
  struct stat buf;
  struct uhuru_file_context file_context;
  enum uhuru_file_context_status context_status;
  enum uhuru_file_status status;
  struct uhuru_scan *scan;
  char file_path[PATH_MAX + 1];
  char *p;

  if (m->enable_permission == 0)  /* permission check is disabled, always allow */
    return write_response(m, event->fd, FAN_ALLOW, NULL, "permission is not activated");

  if (m->my_pid == event->pid)   /* file was opened by myself, always allow */
    return write_response(m, event->fd, FAN_ALLOW, NULL, "event PID is myself");

  /* the 2 following tests could be removed: */
  /* if file descriptor does not refer to a file, read() will fail inside os_mime_type_guess_fd() */
  /* in this case, mime_type will be null, context_status will be error and */
  /* response will be ALLOW */
  if (fstat(event->fd, &buf) < 0)
    return write_response(m, event->fd, FAN_ALLOW, NULL, "stat failed");

  if (!S_ISREG(buf.st_mode))
    return write_response(m, event->fd, FAN_ALLOW, NULL, "fd is not a file");

  p = get_file_path_from_fd(event->fd, file_path, PATH_MAX);

  if (p == NULL)
    return write_response(m, event->fd, FAN_ALLOW, NULL, "cannot get path from file descriptor");

  /* get file scan context */
  context_status = uhuru_file_context_get(&file_context, event->fd, p, m->scan_conf);

  if (context_status) {   /* means file must not be scanned */
    uhuru_file_context_close(&file_context);
    return write_response(m, event->fd, FAN_ALLOW, p, "file type is not scanned");
  }

  /* scan in thread pool */
  g_thread_pool_push(m->thread_pool, uhuru_file_context_clone(&file_context), NULL);

#if 0
  /* scan in this thread */
  scan = uhuru_scan_new(m->uhuru, -1);
  status = uhuru_scan_context(scan, &file_context);
  write_response(m, file_context.fd, file_status_2_response(status), p, NULL);
  uhuru_scan_free(scan);

  uhuru_file_context_close(&file_context);
#endif

  return 0;
}

static void notify_event_process(struct access_monitor *m, struct fanotify_event_metadata *event)
{
  char file_path[PATH_MAX + 1];
  char *p;

  p = get_file_path_from_fd(event->fd, file_path, PATH_MAX);

  /* if (m->flags & MONITOR_LOG_EVENT) */
  /*   fprintf(stderr, "fanotify: path %s", p); */
}

/* Size of buffer to use when reading fanotify events */
/* 8192 is recommended by fanotify man page */
#define FANOTIFY_BUFFER_SIZE 8192

static gboolean access_monitor_fanotify_cb(GIOChannel *source, GIOCondition condition, gpointer data)
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

      if ((event->mask & FAN_OPEN_PERM))
	perm_event_process(m, event);
      else
	notify_event_process(m, event);
    }
  }

  return TRUE;
}

static void inotify_event_log(const struct inotify_event *e)
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

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "fanotify: %s", s->str);

  g_string_free(s, TRUE);
}

static char *inotify_event_full_path(struct access_monitor *m, struct inotify_event *event)
{
  char *dir, *full_path;

  dir = (char *)g_hash_table_lookup(m->wd2path_table, GINT_TO_POINTER(event->wd));

  if (dir == NULL) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "fanotify: cannot lookup directory for watch point %d", event->wd);
    return;
  }

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

static void inotify_process_dir_event(struct access_monitor *m, struct inotify_event *event, const char *what)
{
  char *full_path = inotify_event_full_path(m, event);

  fprintf(stderr, "processing dir event %s path = %s\n", what, full_path);

  free(full_path);
}

static void inotify_event_process(struct access_monitor *m, struct inotify_event *event)
{
  if (!(event->mask & IN_ISDIR))
    return;

#ifdef DEBUG
  inotify_event_log(event);
#endif

  if (event->mask & IN_CREATE && event->mask & IN_ISDIR)
    inotify_process_dir_event(m, event, "create");
  if (event->mask & IN_DELETE && event->mask & IN_ISDIR)
    inotify_process_dir_event(m, event, "delete");
  if (event->mask & IN_MOVE_SELF && event->mask & IN_ISDIR)
    inotify_process_dir_event(m, event, "move self");
  if (event->mask & IN_MOVED_FROM && event->mask & IN_ISDIR)
    inotify_process_dir_event(m, event, "moved from");
  if (event->mask & IN_MOVED_TO && event->mask & IN_ISDIR)
    inotify_process_dir_event(m, event, "move to");
}

/* Size of buffer to use when reading inotify events */
#define INOTIFY_BUFFER_SIZE 8192

static gboolean access_monitor_inotify_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  struct access_monitor *m = (struct access_monitor *)data;
  char event_buffer[INOTIFY_BUFFER_SIZE];
  ssize_t len;

  if ((len = read (m->inotify_fd, event_buffer, INOTIFY_BUFFER_SIZE)) > 0)  {
    char *p;

    p = event_buffer;
    while (p < event_buffer + len) {
      struct inotify_event *event = (struct inotify_event *) p;

      inotify_event_process(m, event);

      p += sizeof(struct inotify_event) + event->len;
    }
  }

  return TRUE;
}
