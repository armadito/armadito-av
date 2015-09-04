#include "libuhuru-config.h"
#include <libuhuru/module.h>

#include "ipc.h"

#include <glib.h>
#include <string.h>
#include <stdlib.h>

static void module_update_info_free(struct uhuru_module_info *info)
{
  struct uhuru_base_info **p;

  if (info->base_infos == NULL)
    return;

  for(p = info->base_infos; *p != NULL; p++)
    free(*p);

  free(info->base_infos);

  info->base_infos = NULL;
}

enum uhuru_update_status uhuru_module_info_update(struct uhuru_module *module, struct uhuru_module_info *info)
{
  info->update_status = UHURU_UPDATE_NON_AVAILABLE;
  memset(&info->update_date, 0, sizeof(struct tm));
  module_update_info_free(info);

  if (module->info_fun != NULL) {
    info->update_status = (*module->info_fun)(module, info);
  }

  return info->update_status;
}

static void ipc_handler_info_module(struct ipc_manager *m, void *data)
{
  struct uhuru_module_info *info;
  int n_bases, i, argc;

  ipc_manager_get_arg_at(m, 0, IPC_INT32, &info->update_status);
  ipc_manager_get_arg_at(m, 1, IPC_INT32, &info->update_date.tm_sec);
  ipc_manager_get_arg_at(m, 2, IPC_INT32, &info->update_date.tm_min);
  ipc_manager_get_arg_at(m, 3, IPC_INT32, &info->update_date.tm_hour);
  ipc_manager_get_arg_at(m, 4, IPC_INT32, &info->update_date.tm_mday);
  ipc_manager_get_arg_at(m, 5, IPC_INT32, &info->update_date.tm_mon);
  ipc_manager_get_arg_at(m, 6, IPC_INT32, &info->update_date.tm_year);

  info->base_infos = g_new0(struct uhuru_base_info *, n_bases + 1);

  n_bases = (ipc_manager_get_argc(m) - 7) / 10 + 1;
  argc = 7;

  for (i = 0; i < n_bases; i++) {
    struct uhuru_base_info *base_info = g_new(struct uhuru_base_info, 1);

    ipc_manager_get_arg_at(m, argc+0, IPC_STRING, (void *)base_info->name);
    ipc_manager_get_arg_at(m, argc+1, IPC_INT32, base_info->date.tm_sec);
    ipc_manager_get_arg_at(m, argc+2, IPC_INT32, base_info->date.tm_min);
    ipc_manager_get_arg_at(m, argc+3, IPC_INT32, base_info->date.tm_hour);
    ipc_manager_get_arg_at(m, argc+4, IPC_INT32, base_info->date.tm_mday);
    ipc_manager_get_arg_at(m, argc+5, IPC_INT32, base_info->date.tm_mon);
    ipc_manager_get_arg_at(m, argc+6, IPC_INT32, base_info->date.tm_year);
    ipc_manager_get_arg_at(m, argc+7, IPC_STRING, (void *)base_info->version);
    ipc_manager_get_arg_at(m, argc+8, IPC_INT32, base_info->signature_count);
    ipc_manager_get_arg_at(m, argc+9, IPC_STRING, (void *)base_info->full_path);

    info->base_infos[i] = base_info;
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
