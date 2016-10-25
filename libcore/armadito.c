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

#include <libarmadito/armadito.h>

#include "armadito-config.h"

#include "module_p.h"
#include "status_p.h"
#include "armadito_p.h"
#include "core/mimetype.h"
#include "core/dir.h"
#include "core/error.h"
#ifdef HAVE_ONDEMAND_MODULE
#include "builtin-modules/ondemandmod.h"
#endif
#ifdef HAVE_ALERT_MODULE
#include "builtin-modules/alert.h"
#endif
#ifdef HAVE_QUARANTINE_MODULE
#include "builtin-modules/quarantine.h"
#endif
#ifdef HAVE_ON_ACCESS_WINDOWS_MODULE
#include "builtin-modules/onaccess_windows.h"
#endif

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct armadito {
	struct module_manager *module_manager;
	struct a6o_conf *conf;
};

static struct armadito *a6o_new(void)
{
	struct armadito *u = g_new(struct armadito, 1);

	u->module_manager = module_manager_new(u);

	return u;
}

static void a6o_free(struct armadito *u)
{
	module_manager_free(u->module_manager);
	g_free(u);
}

static void a6o_add_builtin_modules(struct armadito *u)
{
#ifdef HAVE_ONDEMAND_MODULE
	module_manager_add(u->module_manager, &on_demand_module);
#endif
#ifdef HAVE_ON_ACCESS_WINDOWS_MODULE
	module_manager_add(u->module_manager, &on_access_win_module);
#endif
#ifdef HAVE_ALERT_MODULE
	module_manager_add(u->module_manager, &alert_module);
#endif
#ifdef HAVE_QUARANTINE_MODULE
	module_manager_add(u->module_manager, &quarantine_module);
#endif
}

struct armadito *a6o_open(struct a6o_conf *conf, a6o_error **error)
{
	struct armadito *u;
	const char *modules_dir;

#ifdef HAVE_GTHREAD_INIT
	g_thread_init(NULL);
#endif
	os_mime_type_init();

	u = a6o_new();
	u->conf = conf;
	a6o_add_builtin_modules(u);

	modules_dir = a6o_std_path(MODULES_LOCATION);
	if (modules_dir == NULL) {
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "cannot get modules location, no dynamic loading of modules");
	} else {
		if (module_manager_load_path(u->module_manager, modules_dir, error))
			a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "error during modules load");

		free((void *)modules_dir);
	}

	if (module_manager_init_all(u->module_manager, error))
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "error during modules init");

	if (module_manager_configure_all(u->module_manager, conf, error))
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "error during modules configuration");

	if (module_manager_post_init_all(u->module_manager, error))
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "error during modules post_init");

	return u;
}

struct a6o_conf *a6o_get_conf(struct armadito *u)
{
	return u->conf;
}

struct a6o_module **a6o_get_modules(struct armadito *u)
{
	return module_manager_get_modules(u->module_manager);
}

struct a6o_module *a6o_get_module_by_name(struct armadito *u, const char *name)
{
	return module_manager_get_module_by_name(u->module_manager, name);
}

int a6o_close(struct armadito *u, a6o_error **error)
{
	return module_manager_close_all(u->module_manager, error);
}

#ifdef DEBUG
const char *a6o_debug(struct armadito *u)
{
	struct a6o_module **modv;
	GString *s = g_string_new("");
	const char *ret;

	g_string_append_printf(s, "armadito:\n");

	for (modv = module_manager_get_modules(u->module_manager); *modv != NULL; modv++)
		g_string_append_printf(s, "%s\n", module_debug(*modv));

	ret = s->str;
	g_string_free(s, FALSE);

	return ret;
}
#endif
