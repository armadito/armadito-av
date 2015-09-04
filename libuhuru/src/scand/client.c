#include <libuhuru/module.h>

#include "libuhuru-config.h"
#include "client.h"
#include "lib/ipc.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

struct client {
  int sock;
  struct ipc_manager *manager;
  struct uhuru *uhuru;
};

static void ipc_ping_handler(struct ipc_manager *m, void *data)
{
  struct client *cl = (struct client *)data;

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "ipc_ping_handler");
#endif

  ipc_manager_msg_send(cl->manager, IPC_MSG_ID_PONG, IPC_NONE);
}

static void scan_callback(struct uhuru_report *report, void *callback_data)
{
  struct client *cl = (struct client *)callback_data;

  ipc_manager_msg_send(cl->manager, IPC_MSG_ID_SCAN_FILE, 
		       IPC_STRING, report->path, 
		       IPC_INT32, report->status,
		       IPC_STRING, report->mod_name,
		       IPC_STRING, report->mod_report,
		       IPC_INT32, report->action,
		       IPC_NONE);
}

static void ipc_scan_handler(struct ipc_manager *m, void *data)
{
  struct client *cl = (struct client *)data;
  char *path;
  struct uhuru_scan *scan;

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "ipc_scan_handler");
#endif

  ipc_manager_get_arg_at(m, 0, IPC_STRING, &path);

  scan = uhuru_scan_new(cl->uhuru, path, UHURU_SCAN_RECURSE);

  uhuru_scan_add_callback(scan, scan_callback, cl);

  uhuru_scan_start(scan);

  while (uhuru_scan_run(scan) == UHURU_SCAN_CONTINUE)
    ;

  uhuru_scan_free(scan);

  ipc_manager_msg_send(cl->manager, IPC_MSG_ID_SCAN_END, IPC_NONE);

  if (close(cl->sock) < 0) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "closing socket %d failed (%s)", cl->sock, strerror(errno));
  }

  cl->sock = -1;

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "ipc_scan_handler finished");
#endif
}

static void ipc_info_module(struct ipc_manager *m, struct uhuru_module_info *info)
{
  struct uhuru_base_info **pinfo;

  ipc_manager_msg_begin(m, IPC_MSG_ID_INFO_MODULE);

  ipc_manager_msg_add(m, 
		      IPC_INT32, info->update_status,
		      IPC_INT32, info->update_date.tm_sec,
		      IPC_INT32, info->update_date.tm_min,
		      IPC_INT32, info->update_date.tm_hour,
		      IPC_INT32, info->update_date.tm_mday,
		      IPC_INT32, info->update_date.tm_mon,
		      IPC_INT32, info->update_date.tm_year);

  for(pinfo = info->base_infos; *pinfo != NULL; pinfo++) {
    ipc_manager_msg_add(m, 
			IPC_STRING, (*pinfo)->name,
			IPC_INT32, (*pinfo)->date.tm_sec,
			IPC_INT32, (*pinfo)->date.tm_min,
			IPC_INT32, (*pinfo)->date.tm_hour,
			IPC_INT32, (*pinfo)->date.tm_mday,
			IPC_INT32, (*pinfo)->date.tm_mon,
			IPC_INT32, (*pinfo)->date.tm_year,
			IPC_STRING, (*pinfo)->version,
			IPC_INT32, (*pinfo)->signature_count,
			IPC_STRING, (*pinfo)->full_path);
  }

  ipc_manager_msg_end(m);
}

static void ipc_info_handler(struct ipc_manager *m, void *data)
{
  struct client *cl = (struct client *)data;

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "ipc_info_handler");
#endif


  ipc_manager_msg_send(cl->manager, IPC_MSG_ID_INFO_END, IPC_NONE);
}

struct client *client_new(int client_sock, struct uhuru *uhuru)
{
  struct client *cl = (struct client *)malloc(sizeof(struct client));

  cl->sock = client_sock;
  cl->manager = ipc_manager_new(cl->sock, cl->sock);
  cl->uhuru = uhuru;

  ipc_manager_add_handler(cl->manager, IPC_MSG_ID_PING, ipc_ping_handler, cl);
  ipc_manager_add_handler(cl->manager, IPC_MSG_ID_SCAN, ipc_scan_handler, cl);
  ipc_manager_add_handler(cl->manager, IPC_MSG_ID_INFO, ipc_info_handler, cl);

  return cl;
}

void client_free(struct client *cl)
{
  ipc_manager_free(cl->manager);

  free(cl);
}

int client_process(struct client *cl)
{
  int ret;

  if (cl->sock < 0)
    return 0;

  ret = ipc_manager_receive(cl->manager);

  return ret;
}
