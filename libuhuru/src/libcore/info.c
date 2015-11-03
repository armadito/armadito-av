#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "uhurup.h"
#include "os/string.h"
#include "os/io.h"

#include <assert.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

const char *uhuru_update_status_str(enum uhuru_update_status status)
{
  switch(status) {
#undef M
#define M(S) case S: return #S
    M(UHURU_UPDATE_OK);
    M(UHURU_UPDATE_LATE);
    M(UHURU_UPDATE_CRITICAL);
    M(UHURU_UPDATE_NON_AVAILABLE);
  }

  return "UHURU_UPDATE_UNKNOWN";
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

  return 666;
}

struct uhuru_info *uhuru_info_new(struct uhuru *uhuru)
{
  struct uhuru_info *info = g_new0(struct uhuru_info, 1);
  GArray *g_module_infos;
  struct uhuru_module **modv;

  g_module_infos = g_array_new(TRUE, TRUE, sizeof(struct uhuru_module_info *));

  info->global_status = UHURU_UPDATE_NON_AVAILABLE;

  for (modv = uhuru_get_modules(uhuru); *modv != NULL; modv++) {
    if ((*modv)->info_fun != NULL) {
      enum uhuru_update_status mod_status;
      struct uhuru_module_info *mod_info = g_new0(struct uhuru_module_info, 1);
      
      mod_status = (*(*modv)->info_fun)((*modv), mod_info);

      if (mod_status != UHURU_UPDATE_NON_AVAILABLE) {
	mod_info->name = os_strdup((*modv)->name);
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

void uhuru_info_to_stdout(struct uhuru_info *info)
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
