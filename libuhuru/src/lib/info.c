#include "libuhuru-config.h"
#include <libuhuru/module.h>
#include <libuhuru/info.h>

#include "ipc.h"
#include "uhurup.h"
#include "builtin-modules/remote.h"

#include <assert.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

static int update_status_compare(enum uhuru_update_status s1, enum uhuru_update_status s2)
{
  if (s1 == s2)
    return 0;

  switch(s1) {
  case UHURU_UPDATE_NON_AVAILABLE:
    return -1;
  case UHURU_UPDATE_OK:
    return (s2 == UHURU_UPDATE_NON_AVAILABLE) ? 1 : -1;
  case UHURU_UPDATE_LATE:
    return (s2 == UHURU_UPDATE_NON_AVAILABLE || s2 == UHURU_UPDATE_OK) ? 1 : -1;
  case UHURU_UPDATE_CRITICAL:
    return (s2 == UHURU_UPDATE_NON_AVAILABLE || s2 == UHURU_UPDATE_OK || s2 == UHURU_UPDATE_LATE) ? 1 : -1;
  }
}

static void info_local_init(struct uhuru_info *info, struct uhuru *uhuru)
{
  struct uhuru_module **modv;
  GArray *g_module_infos;

  g_module_infos = g_array_new(TRUE, TRUE, sizeof(struct uhuru_module_info *));

  info->global_status = UHURU_UPDATE_NON_AVAILABLE;

  for (modv = uhuru_get_modules(uhuru); *modv != NULL; modv++) {
    if ((*modv)->info_fun != NULL) {
      enum uhuru_update_status mod_status;
      struct uhuru_module_info *mod_info = g_new0(struct uhuru_module_info, 1);
      
      mod_status = (*(*modv)->info_fun)((*modv), mod_info);

      if (mod_status != UHURU_UPDATE_NON_AVAILABLE) {
	mod_info->name = strdup((*modv)->name);
	mod_info->mod_status = mod_status;
	g_array_append_val(g_module_infos, mod_info);
      } else
	g_free(mod_info);

      if (update_status_compare(info->global_status, mod_status) < 0)
	info->global_status = mod_status;
    }
  }

  info->module_infos = (struct uhuru_module_info **)g_module_infos->data;
  g_array_free(g_module_infos, FALSE);
}

struct ipc_handler_info_data {
  struct uhuru_info *info;
  GArray *g_module_infos;
};

static void ipc_handler_info_module(struct ipc_manager *m, void *data)
{
  struct ipc_handler_info_data *handler_data = (struct ipc_handler_info_data *)data;
  struct uhuru_module_info *mod_info = g_new0(struct uhuru_module_info, 1);
  int n_bases, i, argc;
  char *mod_name, *update_date;

  ipc_manager_get_arg_at(m, 0, IPC_STRING_T, &mod_name);
  mod_info->name = strdup(mod_name);
  ipc_manager_get_arg_at(m, 1, IPC_INT32_T, &mod_info->mod_status);
  ipc_manager_get_arg_at(m, 2, IPC_STRING_T, &update_date);
  mod_info->update_date = strdup(update_date);

  n_bases = (ipc_manager_get_argc(m) - 3) / 5;

  mod_info->base_infos = g_new0(struct uhuru_base_info *, n_bases + 1);

  argc = 3;

  for (i = 0; i < n_bases; i++, argc += 5) {
    struct uhuru_base_info *base_info = g_new(struct uhuru_base_info, 1);
    char *name, *date, *version, *full_path;

    ipc_manager_get_arg_at(m, argc+0, IPC_STRING_T, &name);
    base_info->name = strdup(name);
    ipc_manager_get_arg_at(m, argc+1, IPC_STRING_T, &date);
    base_info->date = strdup(date);
    ipc_manager_get_arg_at(m, argc+2, IPC_STRING_T, &version);
    base_info->version = strdup(version);
    ipc_manager_get_arg_at(m, argc+3, IPC_INT32_T, &base_info->signature_count);
    ipc_manager_get_arg_at(m, argc+4, IPC_STRING_T, &full_path);
    base_info->full_path = strdup(full_path);

    mod_info->base_infos[i] = base_info;
  }

  g_array_append_val(handler_data->g_module_infos, mod_info);
}

static void ipc_handler_info_end(struct ipc_manager *m, void *data)
{
  struct ipc_handler_info_data *handler_data = (struct ipc_handler_info_data *)data;
  struct uhuru_info *info = handler_data->info;
  GArray *g_module_infos = handler_data->g_module_infos;

  info->module_infos = (struct uhuru_module_info **)g_module_infos->data;
  g_array_free(g_module_infos, FALSE);

  ipc_manager_get_arg_at(m, 0, IPC_INT32_T, &info->global_status);
}

static int info_remote_init(struct uhuru_info *info, struct uhuru *uhuru)
{
  struct uhuru_module *remote_module;
  const char *sock_dir;
  GString *sock_path;
  int sock;
  struct ipc_manager *manager;
  struct ipc_handler_info_data *data;

  remote_module = uhuru_get_module_by_name(uhuru, "remote");
  assert(remote_module != NULL);

  sock_dir = remote_module_get_sock_dir(remote_module);
  assert(sock_dir != NULL);

  sock_path = g_string_new(sock_dir);
  g_string_append_printf(sock_path, "/uhuru-%s", getenv("USER"));

  sock = client_socket_create(sock_path->str, 10);
  if (sock < 0)
    return -1;

  g_string_free(sock_path, TRUE);

  manager = ipc_manager_new(sock, sock);

  data = g_new(struct ipc_handler_info_data, 1);
  data->info = info;
  data->g_module_infos = g_array_new(TRUE, TRUE, sizeof(struct uhuru_module_info *));

  ipc_manager_add_handler(manager, IPC_MSG_ID_INFO_MODULE, ipc_handler_info_module, data);
  ipc_manager_add_handler(manager, IPC_MSG_ID_INFO_END, ipc_handler_info_end, data);

  ipc_manager_msg_send(manager, IPC_MSG_ID_INFO, IPC_NONE_T);

  while (ipc_manager_receive(manager) > 0)
    ;

  g_free(data);
}

struct uhuru_info *uhuru_info_new(struct uhuru *uhuru)
{
  struct uhuru_info *info = g_new0(struct uhuru_info, 1);

  if (uhuru_is_remote(uhuru))
    info_remote_init(info, uhuru);
  else
    info_local_init(info, uhuru);

  return info;
}

void uhuru_info_free(struct uhuru_info *info)
{
  struct uhuru_module_info **m;

  if (info->module_infos != NULL) {
    for(m = info->module_infos; *m != NULL; m++) {
      free((void *)(*m)->name);
      free((void *)(*m)->update_date);

      if ((*m)->base_infos != NULL) {
	struct uhuru_base_info **b;

	for(b = (*m)->base_infos; *b != NULL; b++) {
	  free((void *)(*b)->name);
	  free((void *)(*b)->date);
	  free((void *)(*b)->version);
	  free((void *)(*b)->full_path);

	  free(*b);
	}

	free((*m)->base_infos);
      }

      g_free(*m);
    }

    g_free(info->module_infos);
  }

  g_free(info);
}

void info_to_stdout(struct uhuru_info *info)
{
     struct uhuru_module_info **m;
     struct uhuru_base_info **b;

     fprintf(stdout, "--- Uhuru_info --- \n");
     fprintf(stdout, "Update global status : %d\n", info->global_status);
     if (info->module_infos != NULL) {
	     for(m = info->module_infos; *m != NULL; m++){
		fprintf(stdout, "Module %s \n", (*m)->name );
		fprintf(stdout, "- Update date : %s \n", (*m)->update_date );
		fprintf(stdout, "- Update status : %d\n", (*m)->mod_status);

		if ((*m)->base_infos != NULL) {
		  for(b = (*m)->base_infos; *b != NULL; b++){
		      fprintf(stdout, "-- Base %s \n", (*b)->name );
		      fprintf(stdout, "--- Update date : %s \n", (*b)->date );
		      fprintf(stdout, "--- Version : %s \n", (*b)->version );
		      fprintf(stdout, "--- Signature count : %d \n", (*b)->signature_count );
		      fprintf(stdout, "--- Full path : %s \n", (*b)->full_path );
		  }
		}
	   }
     }
}

#ifdef DEBUG
const char *uhuru_base_info_debug(struct uhuru_base_info *info)
{
  GString *s = g_string_new("");
  const char *ret;

  g_string_append_printf(s, "Base '%s':\n", info->name);
  /* g_string_append_printf(s, "  date              %s\n", info->date); */
  g_string_append_printf(s, "  version           %s\n", info->version);
  g_string_append_printf(s, "  signature_count   %d\n", info->signature_count);
  g_string_append_printf(s, "  full_path         %s\n", info->full_path);

  ret = s->str;
  g_string_free(s, FALSE);

  return ret;
}
#endif
