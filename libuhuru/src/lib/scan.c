#include "libuhuru-config.h"
#include <libuhuru/module.h>
#include <libuhuru/scan.h>
#include "conf.h"
#include "dir.h"
#include "modulep.h"
#include "ipc.h"
#include "statusp.h"
#include "uhurup.h"
#include "unixsock.h"
#include "builtin-modules/alert.h"
#include "builtin-modules/quarantine.h"
#include "builtin-modules/remote.h"

#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <magic.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct callback_entry {
  uhuru_scan_callback_t callback;
  void *callback_data;
};

struct local_scan {
  GThreadPool *thread_pool;  
  GPrivate *private_magic_key;
};

struct remote_scan {
  char *sock_path;
  int sock;
  struct ipc_manager *manager;
};

struct uhuru_scan {
  struct uhuru *uhuru;
  const char *path;
  enum uhuru_scan_flags flags;
  GArray *callbacks;
  int is_remote;

  union {
    struct local_scan local;
    struct remote_scan remote;
  };
};

static void uhuru_scan_call_callbacks(struct uhuru_scan *scan, struct uhuru_report *report);

/* 
 * **************************************************
 * Local version
 * *************************************************
 */

static void local_scan_init(struct uhuru_scan *scan)
{
  struct uhuru_module *alert_module, *quarantine_module;

  scan->local.thread_pool = NULL;
  scan->local.private_magic_key = NULL;

  alert_module = uhuru_get_module_by_name(scan->uhuru, "alert");
  uhuru_scan_add_callback(scan, alert_callback, alert_module->data);

  quarantine_module = uhuru_get_module_by_name(scan->uhuru, "quarantine");
  uhuru_scan_add_callback(scan, quarantine_callback, quarantine_module->data);
}

static enum uhuru_file_status local_scan_apply_modules(const char *path, const char *mime_type, struct uhuru_module **modules,  struct uhuru_report *report)
{
  enum uhuru_file_status current_status = UHURU_UNDECIDED;

  for (; *modules != NULL; modules++) {
    struct uhuru_module *mod = *modules;
    enum uhuru_file_status mod_status;
    char *mod_report = NULL;

    if (mod->status != UHURU_MOD_OK)
      continue;

    mod_status = (*mod->scan_fun)(mod, path, mime_type, &mod_report);

#if 0
#ifdef DEBUG
    g_log(NULL, G_LOG_LEVEL_DEBUG, "module %s: scanning %s -> %s", mod->name, path, uhuru_file_status_str(mod_status));
#endif
#endif

    if (uhuru_file_status_cmp(current_status, mod_status) < 0) {
      current_status = mod_status;
      uhuru_report_change(report, mod_status, (char *)mod->name, mod_report);
    } else if (mod_report != NULL)
      free(mod_report);

    if (current_status == UHURU_WHITE_LISTED || current_status == UHURU_MALWARE)
      break;
  }

  return current_status;
}

static void local_scan_file(struct uhuru_scan *scan, magic_t magic, const char *path)
{
  enum uhuru_file_status status;
  struct uhuru_report report;
  struct uhuru_module **modules;
  const char *mime_type;

  uhuru_report_init(&report, path);

  modules = uhuru_get_applicable_modules(scan->uhuru, magic, path, &mime_type);

  if (modules == NULL)
    report.status = UHURU_UNKNOWN_FILE_TYPE;
  else
    status = local_scan_apply_modules(path, mime_type, modules, &report);

  uhuru_scan_call_callbacks(scan, &report);

  uhuru_report_destroy(&report);

  free((void *)mime_type);
}

/* Unfortunately, libmagic is not thread-safe. */
/* We create a new magic_t for each thread, and keep it  */
/* in thread's private data, so that it is created only once. */
static void magic_destroy_notify(gpointer data)
{
  magic_close((magic_t)data);
}

static magic_t get_private_magic(struct uhuru_scan *scan)
{
  magic_t m = (magic_t)g_private_get(scan->local.private_magic_key);

  if (m == NULL) {
    m = magic_open(MAGIC_MIME_TYPE);
    magic_load(m, NULL);

    g_private_set(scan->local.private_magic_key, (gpointer)m);
  }

  return m;
}

static void local_scan_entry_thread_fun(gpointer data, gpointer user_data)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)user_data;
  char *path = (char *)data;

  local_scan_file(scan, get_private_magic(scan), path);

  free(path);
}

static void local_scan_entry(const char *full_path, enum dir_entry_flag flags, int entry_errno, void *data)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)data;

  if (flags & DIR_ENTRY_IS_ERROR) {
    struct uhuru_report report;

    uhuru_report_init(&report, full_path);

    report.status = UHURU_IERROR;
	
	#ifdef WIN32
		//report.mod_report = _strdup(strerror(entry_errno));
		report.mod_report = _strdup("TODO::replace strerror(entry_errno)");
	#else
		report.mod_report = strdup(strerror(entry_errno));
	#endif
    
    uhuru_scan_call_callbacks(scan, &report);

    uhuru_report_destroy(&report);
  }

  if (!(flags & DIR_ENTRY_IS_REG))
    return;

  if (scan->flags & UHURU_SCAN_THREADED)
	
	#ifdef WIN32
		g_thread_pool_push(scan->local.thread_pool, (gpointer)_strdup(full_path), NULL);
	#else
		g_thread_pool_push(scan->local.thread_pool, (gpointer)strdup(full_path), NULL);
	#endif

  else
    local_scan_file(scan, NULL, full_path);
}

static int get_max_threads(void)
{
  return 8;
}

static enum uhuru_scan_status local_scan_start(struct uhuru_scan *scan)
{
  if (scan->flags & UHURU_SCAN_THREADED) {
    scan->local.thread_pool = g_thread_pool_new(local_scan_entry_thread_fun, scan, get_max_threads(), FALSE, NULL);
    scan->local.private_magic_key = g_private_new(magic_destroy_notify);
  }

  return UHURU_SCAN_OK;
}

static enum uhuru_scan_status local_scan_run(struct uhuru_scan *scan)
{
  struct stat sb;

  if (stat(scan->path, &sb) == -1) {
    perror("stat");
    /* exit(EXIT_FAILURE); */
  }

  if (S_ISREG(sb.st_mode)) {
    if (scan->flags & UHURU_SCAN_THREADED)
		#ifdef WIN32
			g_thread_pool_push(scan->local.thread_pool, (gpointer)_strdup(scan->path), NULL);
		#else
			g_thread_pool_push(scan->local.thread_pool, (gpointer)strdup(scan->path), NULL);
		#endif
    else
      local_scan_file(scan, NULL, scan->path);
  } else if (S_ISDIR(sb.st_mode)) {
    int recurse = scan->flags & UHURU_SCAN_RECURSE;

    dir_map(scan->path, recurse, local_scan_entry, scan);
  }

  if (scan->flags & UHURU_SCAN_THREADED)
    g_thread_pool_free(scan->local.thread_pool, FALSE, TRUE);

  return UHURU_SCAN_COMPLETED;
}

/* 
 * **************************************************
 * Remote version
 * *************************************************
 */

static void remote_scan_init(struct uhuru_scan *scan)
{
  struct uhuru_module *remote_module;
  const char *sock_dir;
  GString *sock_path;
  char * envar = NULL;
  ssize_t size =0 ;
  errno_t err;

  remote_module = uhuru_get_module_by_name(scan->uhuru, "remote");
  assert(remote_module != NULL);

  sock_dir = remote_module_get_sock_dir(remote_module);
  assert(sock_dir != NULL);

  sock_path = g_string_new(sock_dir);

#ifdef WIN32
	err = _dupenv_s(&envar,&size, "USER");
	if (err == 0)
		g_string_append_printf(sock_path, "/uhuru-%s", &envar);
#else
	g_string_append_printf(sock_path, "/uhuru-%s", getenv("USER"));
#endif

  scan->remote.sock_path = sock_path->str;
  g_string_free(sock_path, FALSE);
}

static void remote_scan_handler_scan_file(struct ipc_manager *m, void *data)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)data;
  char *path;
  gint32 status;
  char *mod_name;
  char *x_status;
  gint32 action;
  struct uhuru_report report;

  ipc_manager_get_arg_at(m, 0, IPC_STRING_T, &path);
  ipc_manager_get_arg_at(m, 1, IPC_INT32_T, &status);
  ipc_manager_get_arg_at(m, 2, IPC_STRING_T, &mod_name);
  ipc_manager_get_arg_at(m, 3, IPC_STRING_T, &x_status);
  ipc_manager_get_arg_at(m, 4, IPC_INT32_T, &action);

  uhuru_report_init(&report, path);

  report.status = (enum uhuru_file_status)status;
  report.action = (enum uhuru_action)action;
  report.mod_name = mod_name;
  if (x_status != NULL) {
	#ifdef WIN32
		report.mod_report = _strdup(x_status);
	#else
		report.mod_report = strdup(x_status);
	#endif
  }
    

  uhuru_scan_call_callbacks(scan, &report);

  uhuru_report_destroy(&report);
}

static void remote_scan_handler_scan_end(struct ipc_manager *m, void *data)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)data;

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "remote scan end");
#endif
  
#if 0
#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "remote scan end, closing socket %d", scan->remote.sock);
#endif
  
  if (close(scan->remote.sock) < 0) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "closing socket %d failed (%s)", scan->remote.sock, strerror(errno));
  }

  scan->remote.sock = -1;
#endif
}

static enum uhuru_file_status remote_scan_start(struct uhuru_scan *scan)
{
  scan->remote.sock = client_socket_create(scan->remote.sock_path, 10);
  if (scan->remote.sock < 0)
    return UHURU_IERROR;

  scan->remote.manager = ipc_manager_new(scan->remote.sock, scan->remote.sock);

  ipc_manager_add_handler(scan->remote.manager, IPC_MSG_ID_SCAN_FILE, remote_scan_handler_scan_file, scan);
  ipc_manager_add_handler(scan->remote.manager, IPC_MSG_ID_SCAN_END, remote_scan_handler_scan_end, scan);

  ipc_manager_msg_send(scan->remote.manager, IPC_MSG_ID_SCAN, IPC_STRING_T, scan->path, IPC_NONE_T);
}

static enum uhuru_scan_status remote_scan_run(struct uhuru_scan *scan)
{
  if (ipc_manager_receive(scan->remote.manager) <= 0)
    return UHURU_SCAN_COMPLETED;

  return UHURU_SCAN_CONTINUE;
}

/* 
 * **************************************************
 * common API
 * *************************************************
 */

struct uhuru_scan *uhuru_scan_new(struct uhuru *uhuru, const char *path, enum uhuru_scan_flags flags)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)malloc(sizeof(struct uhuru_scan));

  scan->uhuru = uhuru;
  scan->path = (const char *)realpath(path, NULL);
  if (scan->path == NULL) {
    perror("realpath");
    free(scan);
    return NULL;
  }

  scan->flags = flags;
  scan->callbacks = g_array_new(FALSE, FALSE, sizeof(struct callback_entry));
  scan->is_remote = uhuru_is_remote(uhuru);

  if (scan->is_remote)
    remote_scan_init(scan);
  else
    local_scan_init(scan);

  return scan;
}

void uhuru_scan_add_callback(struct uhuru_scan *scan, uhuru_scan_callback_t callback, void *callback_data)
{
  struct callback_entry entry;

  entry.callback = callback;
  entry.callback_data = callback_data;

  g_array_append_val(scan->callbacks, entry);
}

static void uhuru_scan_call_callbacks(struct uhuru_scan *scan, struct uhuru_report *report)
{
  int i;

  for(i = 0; i < scan->callbacks->len; i++) {
    struct callback_entry *entry = &g_array_index(scan->callbacks, struct callback_entry, i);
    uhuru_scan_callback_t callback = entry->callback;

    (*callback)(report, entry->callback_data);
  }
}

int uhuru_scan_get_poll_fd(struct uhuru_scan *scan)
{
  if (!scan->is_remote) {
    g_log(NULL, G_LOG_LEVEL_ERROR, "cannot call uhuru_scan_get_poll_fd() for a local scan");
    
    return -1;
  }

  return scan->remote.sock;
}

enum uhuru_scan_status uhuru_scan_start(struct uhuru_scan *scan)
{
  return (scan->is_remote) ? remote_scan_start(scan) : local_scan_start(scan);
}

enum uhuru_scan_status uhuru_scan_run(struct uhuru_scan *scan)
{
  return (scan->is_remote) ? remote_scan_run(scan) : local_scan_run(scan);
}

void uhuru_scan_free(struct uhuru_scan *scan)
{
  free((char *)scan->path);

  g_array_free(scan->callbacks, TRUE);

  free(scan);
}

