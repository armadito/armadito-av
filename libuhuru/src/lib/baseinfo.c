#include "libuhuru-config.h"
#include <libuhuru/module.h>

#include <glib.h>

static void uhuru_base_info_free(struct uhuru_module *module)
{
  struct uhuru_base_info **p;

  for(p = module->base_infos; *p != NULL; p++)
    free(*p);

  free(module->base_infos);

  module->base_infos = NULL;
}

enum uhuru_update_status uhuru_base_info_update(struct uhuru_module *module)
{
  if (module->update_check_fun == NULL)
    return UHURU_UPDATE_NON_AVAILABLE;

  if (module->base_infos != NULL)
    uhuru_base_info_free(module);

  module->update_status = (*module->update_check_fun)(module);

  if (module->update_status == UHURU_UPDATE_NON_AVAILABLE)
    uhuru_base_info_free(module);

  return module->update_status;
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
