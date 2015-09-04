#include "libuhuru-config.h"
#include <libuhuru/module.h>

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

  if (module->update_check_fun != NULL) {
    info->update_status = (*module->update_check_fun)(module, info);
  }

  return info->update_status;
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
