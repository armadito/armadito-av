#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "conf.h"
#include "modulep.h"
#include "statusp.h"
#include "uhurup.h"
#include "os/string.h"
#include "os/mimetype.h"
#include "os/string.h"
#include "os/dir.h"
#ifdef HAVE_ALERT_MODULE
#include "builtin-modules/alert.h"
#endif
#ifdef HAVE_QUARANTINE_MODULE
#include "builtin-modules/quarantine.h"
#endif
#include "builtin-modules/ondemandmod.h"
#ifdef HAVE_ON_ACCESS_WINDOWS_MODULE
#include "builtin-modules/onaccess_windows.h"
#endif

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern struct uhuru_module on_access_module;

struct uhuru {
  struct module_manager *module_manager;
  GHashTable *mime_type_table;
};

static struct uhuru *uhuru_new(void)
{
  struct uhuru *u = g_new(struct uhuru, 1);

  u->module_manager = module_manager_new(u);

  u->mime_type_table = g_hash_table_new(g_str_hash, g_str_equal);

  return u;
}

static void uhuru_free(struct uhuru *u)
{
  module_manager_free(u->module_manager);
  g_hash_table_destroy(u->mime_type_table);
  g_free(u);
}

struct uhuru *uhuru_open(uhuru_error **error)
{
  struct uhuru *u;
  char * dirpath = NULL;
  char * confdir = NULL;
  char * modulesdir = NULL;
  char * conffile = "\\uhuru.conf";
  int len = 0;

#ifdef HAVE_GTHREAD_INIT
  g_thread_init(NULL);
#endif

  os_mime_type_init();

  u = uhuru_new();

  module_manager_add(u->module_manager, &on_demand_module);
#ifdef HAVE_ON_ACCESS_WINDOWS_MODULE
  module_manager_add(u->module_manager, &on_access_win_module);
#endif

#ifdef HAVE_ALERT_MODULE
  module_manager_add(u->module_manager, &alert_module);
#endif
#ifdef HAVE_QUARANTINE_MODULE
  module_manager_add(u->module_manager, &quarantine_module);
#endif

#ifdef WIN32

	dirpath = GetBinaryDirectory( );
	if (dirpath == NULL) {
		goto error;
	}

	len = strnlen_s(dirpath, _MAX_PATH) + strnlen_s(LIBUHURU_MODULES_PATH,_MAX_PATH) +1;
	modulesdir = (char*)calloc(len+1,sizeof(char));
	modulesdir[len] = '\0';

	memcpy_s(modulesdir,len,dirpath,strnlen_s(dirpath, _MAX_PATH));
	modulesdir[strnlen_s(dirpath, _MAX_PATH)] = '\\';
	memcpy_s(modulesdir+strnlen_s(dirpath, _MAX_PATH)+1,len,LIBUHURU_MODULES_PATH,strnlen_s(LIBUHURU_MODULES_PATH, _MAX_PATH));


	if (module_manager_load_path(u->module_manager, modulesdir, error))
		goto error;

	free(modulesdir);

#else
  if (module_manager_load_path(u->module_manager, LIBUHURU_MODULES_PATH, error))
    goto error;
#endif

  if (module_manager_init_all(u->module_manager, error))
    goto error;

  /* FIXME: error checking */
#ifdef WIN32

	// build the complete conf directory path.
	if (dirpath == NULL) {
		goto error;
	}

	len = strnlen_s(dirpath, _MAX_PATH) + strnlen_s(LIBUHURU_CONF_DIR,_MAX_PATH) + strnlen_s(conffile,_MAX_PATH)+1;
	confdir = (char*)calloc(len+1,sizeof(char));
	confdir[len] = '\0';

	memcpy_s(confdir,len,dirpath,strnlen_s(dirpath, _MAX_PATH));
	confdir[strnlen_s(dirpath, _MAX_PATH)] = '\\';
	memcpy_s(confdir+strnlen_s(dirpath, _MAX_PATH)+1,len,LIBUHURU_CONF_DIR,strnlen_s(LIBUHURU_CONF_DIR, _MAX_PATH));
	memcpy_s(confdir+strnlen_s(dirpath, _MAX_PATH)+strnlen_s(LIBUHURU_CONF_DIR, _MAX_PATH)+1,len,conffile,strnlen_s(conffile, _MAX_PATH));
 
  printf("Conf_file = %s ", confdir);
  conf_load_file(u, confdir);
  //conf_load_path(u, LIBUHURU_CONF_DIR "/conf.d");

  free(dirpath);
  free(confdir);

#else
  conf_load_file(u, LIBUHURU_CONF_DIR "/uhuru.conf");
  conf_load_path(u, LIBUHURU_CONF_DIR "/conf.d");
#endif

  if (module_manager_post_init_all(u->module_manager, error))
    goto error;

  return u;

 error:
  uhuru_free(u);
  return NULL;
}

struct uhuru_module **uhuru_get_modules(struct uhuru *u)
{
  return module_manager_get_modules(u->module_manager);
}

struct uhuru_module *uhuru_get_module_by_name(struct uhuru *u, const char *module_name)
{
  struct uhuru_module **modv;

  for (modv = module_manager_get_modules(u->module_manager); *modv != NULL; modv++)
    if (!strcmp((*modv)->name, module_name))
      return *modv;

  return NULL;
}

int uhuru_close(struct uhuru *u, uhuru_error **error)
{
  return module_manager_close_all(u->module_manager, error);
}

#ifdef DEBUG
static void mod_print_name(gpointer data, gpointer user_data)
{
  struct uhuru_module *mod = (struct uhuru_module *)data;

  printf("%s ", mod->name);
}

static void print_mime_type_entry(gpointer key, gpointer value, gpointer user_data)
{
  GString *s = (GString *)user_data;
  GArray *modules = (GArray *)value;
  struct uhuru_module **modv;

  g_string_append_printf(s, "    mimetype: %s handled by modules:", (char *)key);
 
  for (modv = (struct uhuru_module **)modules->data; *modv != NULL; modv++)
    g_string_append_printf(s, " %s", (*modv)->name);

  g_string_append_printf(s, "\n");
}

const char *uhuru_debug(struct uhuru *u)
{
  struct uhuru_module **modv;
  GString *s = g_string_new("");
  const char *ret;

  g_string_append_printf(s, "Uhuru:\n");

  for (modv = module_manager_get_modules(u->module_manager); *modv != NULL; modv++)
    g_string_append_printf(s, "%s\n", module_debug(*modv));

  g_string_append_printf(s, "  mime types:\n");
  g_hash_table_foreach(u->mime_type_table, print_mime_type_entry, s);

  ret = s->str;
  g_string_free(s, FALSE);

  return ret;
}
#endif
