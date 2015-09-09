#include "libuhuru-config.h"
#include <libuhuru/module.h>
#include <libuhuru/info.h>

#include "ipc.h"
#include "uhurup.h"

#include <glib.h>
#include <string.h>
#include <stdlib.h>

static void ipc_handler_info_module(struct ipc_manager *m, void *data)
{
  struct uhuru_module_info *info;
  int n_bases, i, argc;

  ipc_manager_get_arg_at(m, 0, IPC_INT32_T, &info->mod_status);
  ipc_manager_get_arg_at(m, 1, IPC_STRING_T, &info->update_date);

  n_bases = (ipc_manager_get_argc(m) - 2) / 5 + 1;

  info->base_infos = g_new0(struct uhuru_base_info *, n_bases + 1);

  argc = 2;

  for (i = 0; i < n_bases; i++) {
    struct uhuru_base_info *base_info = g_new(struct uhuru_base_info, 1);

    ipc_manager_get_arg_at(m, argc+0, IPC_STRING_T, &base_info->name);
    ipc_manager_get_arg_at(m, argc+1, IPC_STRING_T, &base_info->date);
    ipc_manager_get_arg_at(m, argc+2, IPC_STRING_T, &base_info->version);
    ipc_manager_get_arg_at(m, argc+3, IPC_INT32_T, &base_info->signature_count);
    ipc_manager_get_arg_at(m, argc+4, IPC_STRING_T, &base_info->full_path);

    info->base_infos[i] = base_info;
  }

}

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

static void info_remote_init(struct uhuru_info *info, struct uhuru *uhuru)
{
}

struct uhuru_info *uhuru_info_new(struct uhuru *uhuru)
{
  struct uhuru_info *info = g_new(struct uhuru_info, 1);

  if (uhuru_is_remote(uhuru))
    info_remote_init(info, uhuru);
  else
    info_local_init(info, uhuru);

  return info;
}

void uhuru_info_free(struct uhuru_info *info)
{
  if (info->module_infos != NULL) {
    struct uhuru_module_info **m;

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

      free(*m);
    }

    free(info->module_infos);
  }

  free(info);
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
