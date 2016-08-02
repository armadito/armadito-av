/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "libarmadito-config.h"

#include <libarmadito.h>

#include "armaditop.h"
#include "os/string.h"
#include "os/io.h"

#include <assert.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

const char *a6o_update_status_str(enum a6o_update_status status)
{
	switch(status) {
#undef M
#define M(S) case S: return #S
		M(ARMADITO_UPDATE_OK);
		M(ARMADITO_UPDATE_LATE);
		M(ARMADITO_UPDATE_CRITICAL);
		M(ARMADITO_UPDATE_NON_AVAILABLE);
	}

	return "ARMADITO_UPDATE_UNKNOWN";
}


static int update_status_compare(enum a6o_update_status s1, enum a6o_update_status s2)
{
	if (s1 == s2)
		return 0;

	switch(s1) {
	case ARMADITO_UPDATE_NON_AVAILABLE:
		return -1;
	case ARMADITO_UPDATE_OK:
		return (s2 == ARMADITO_UPDATE_NON_AVAILABLE) ? 1 : -1;
	case ARMADITO_UPDATE_LATE:
		return (s2 == ARMADITO_UPDATE_NON_AVAILABLE || s2 == ARMADITO_UPDATE_OK) ? 1 : -1;
	case ARMADITO_UPDATE_CRITICAL:
		return (s2 == ARMADITO_UPDATE_NON_AVAILABLE || s2 == ARMADITO_UPDATE_OK || s2 == ARMADITO_UPDATE_LATE) ? 1 : -1;
	}

	return 666;
}

struct a6o_info *a6o_info_new(struct armadito *armadito)
{
	struct a6o_info *info = malloc(sizeof(struct a6o_info));
	GArray *g_module_infos;
	struct a6o_module **modv;

	info->global_status = ARMADITO_UPDATE_NON_AVAILABLE;
	info->global_update_ts = 0;

	g_module_infos = g_array_new(TRUE, TRUE, sizeof(struct a6o_module_info *));

	for (modv = a6o_get_modules(armadito); *modv != NULL; modv++) {
		struct a6o_module *mod = *modv;

		if (mod->info_fun != NULL) {
			enum a6o_update_status mod_status;
			struct a6o_module_info *mod_info = malloc(sizeof(struct a6o_module_info));

			mod_info->name = NULL;
			mod_info->mod_update_ts = 0;
			mod_info->base_infos = NULL;

			mod_status = (*mod->info_fun)(mod, mod_info);

			if (mod_status != ARMADITO_UPDATE_NON_AVAILABLE) {
				mod_info->name = os_strdup(mod->name);
				mod_info->mod_status = mod_status;
				g_array_append_val(g_module_infos, mod_info);
			} else
				free(mod_info);

			if (update_status_compare(info->global_status, mod_status) < 0)
				info->global_status = mod_status;

			if (mod_info->mod_update_ts > info->global_update_ts)
				info->global_update_ts = mod_info->mod_update_ts;
		}
	}

	info->module_infos = (struct a6o_module_info **)g_module_infos->data;
	g_array_free(g_module_infos, FALSE);

	return info;
}

void a6o_info_free(struct a6o_info *info)
{
	struct a6o_module_info **mod_infov;

	if (info->module_infos != NULL) {
		for(mod_infov = info->module_infos; *mod_infov != NULL; mod_infov++) {
			struct a6o_module_info *mod_info = *mod_infov;

			free((void *)mod_info->name);

			if (mod_info->base_infos != NULL) {
				struct a6o_base_info **b;

				for(b = mod_info->base_infos; *b != NULL; b++) {
					free((void *)(*b)->name);
					free((void *)(*b)->version);
					free((void *)(*b)->full_path);

					free(*b);
				}

				free(mod_info->base_infos);
			}

			free(mod_info);
		}

		free(info->module_infos);
	}

	free(info);
}

#if 0
void a6o_info_to_stdout(struct a6o_info *info)
{
	struct a6o_module_info **m;
	struct a6o_base_info **b;

	fprintf(stdout, "--- Armadito_info --- \n");
	fprintf(stdout, "Update global status : %d\n", info->global_status);
	if (info->module_infos != NULL) {
		for(m = info->module_infos; *m != NULL; m++){
			fprintf(stdout, "Module %s \n", (*m)->name );
			fprintf(stdout, "- Update timestamp : %s \n", (*m)->update_timestamp);
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
const char *a6o_base_info_debug(struct a6o_base_info *info)
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
#endif

