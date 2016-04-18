#include <libarmadito.h>
#include "libarmadito-config.h"

#include "os/string.h"
#include "armaditop.h"

#include <glib.h>
#include <stdlib.h>

struct a6o_scan_conf {
	const char *name;
	size_t max_file_size;

	GArray *mime_types;
	GArray *modules;
	GHashTable *mime_type_cache;

	/* a GArray and not a GPtrArray because GArray can be automatically NULL terminated */
	GArray *directories_white_list;
};

/* macros for easy access to GArray */
#define modules(c) ((struct a6o_module **)((c)->modules->data))
#define mime_types(c) ((const char **)((c)->mime_types->data))
#define directories_white_list(c) ((const char **)((c)->directories_white_list->data))

static struct a6o_scan_conf *a6o_scan_conf_new(const char *name)
{
	struct a6o_scan_conf *c = malloc(sizeof(struct a6o_scan_conf));

	c->name = os_strdup(name);
	c->max_file_size = 0;

	c->mime_types = g_array_new(TRUE, TRUE, sizeof(const char *));
	c->modules = g_array_new(TRUE, TRUE, sizeof(struct a6o_module *));
	c->mime_type_cache = g_hash_table_new(g_str_hash, g_str_equal);

	c->directories_white_list = g_array_new(TRUE, TRUE, sizeof(const char *));

	return c;
}

static struct a6o_scan_conf *on_demand_conf = NULL;
static struct a6o_scan_conf *on_access_conf = NULL;

static struct a6o_scan_conf *get_conf(struct a6o_scan_conf **pc, const char *name)
{
	if (*pc == NULL)
		*pc = a6o_scan_conf_new(name);

	return *pc;
}

struct a6o_scan_conf *a6o_scan_conf_on_demand(void)
{
	return get_conf(&on_demand_conf, "on-demand scan configuration");
}

struct a6o_scan_conf *a6o_scan_conf_on_access(void)
{
	return get_conf(&on_access_conf, "on-access scan configuration");
}

void a6o_scan_conf_white_list_directory(struct a6o_scan_conf *c, const char *path)
{
	const char *cp = os_strdup(path);

	g_array_append_val(c->directories_white_list, cp);
}

static int strprefix(const char *s, const char *prefix)
{
	while (*prefix && *s && *prefix++ == *s++)
		;

	if (*prefix == '\0')
		return *s == '\0' || *s == '/' || *s == '\\';

	return 0;
}

int a6o_scan_conf_is_white_listed(struct a6o_scan_conf *c, const char *path)
{
	const char **p = directories_white_list(c);

	while (*p != NULL) {
		if (strprefix(path, *p))
			return 1;
		p++;
	}

	return 0;
}

void a6o_scan_conf_add_mime_type(struct a6o_scan_conf *c, const char *mime_type)
{
	const char *p = os_strdup(mime_type);

	g_array_append_val(c->mime_types, p);
}

void a6o_scan_conf_add_module(struct a6o_scan_conf *c, const char *module_name, struct armadito *u)
{
	struct a6o_module *mod = a6o_get_module_by_name(u, module_name);

	if (mod == NULL) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, "scan configuration: no module '%s'", module_name);
		return;
	}

	g_array_append_val(c->modules, mod);
}

static int mime_type_contains(const char **p_mime_type, const char *mime_type)
{
	while (*p_mime_type != NULL) {
		if (!strcmp(*p_mime_type, mime_type)
			|| !strcmp(*p_mime_type, "*"))
			return 1;
		p_mime_type++;
	}

	return 0;
}

static struct a6o_module **build_module_array(struct a6o_scan_conf *c, const char *mime_type)
{
	GArray *modules = g_array_new(TRUE, TRUE, sizeof(struct a6o_module *));
	struct a6o_module **p_module;

	for(p_module = modules(c); *p_module != NULL; p_module++) {
		if (mime_type_contains((*p_module)->supported_mime_types, mime_type))
			g_array_append_val(modules, *p_module);
	}

	/* did we add at least one module ? */
	if (modules->len > 0)
		return (struct a6o_module **)g_array_free(modules, FALSE);

	/* no module was added! */
	g_array_free(modules, TRUE);
	return NULL;
}

static struct a6o_module **get_applicable_modules(struct a6o_scan_conf *c, const char *mime_type)
{
	struct a6o_module **modules = NULL;

	if (g_hash_table_lookup_extended(c->mime_type_cache, mime_type, NULL, (gpointer *)&modules))
		return modules;

	if (!mime_type_contains(mime_types(c), mime_type)) {
		g_hash_table_insert(c->mime_type_cache, (gpointer)(os_strdup(mime_type)), NULL);
		return NULL;
	}

	modules = build_module_array(c, mime_type);
	g_hash_table_insert(c->mime_type_cache, (gpointer)(os_strdup(mime_type)), modules);

	return modules;
}

struct a6o_module **a6o_scan_conf_get_applicable_modules(struct a6o_scan_conf *c, const char *mime_type)
{
	struct a6o_module **modules = get_applicable_modules(c, mime_type);

	if (modules == NULL) {
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "%s: mime-type '%s' -> NULL", c->name, mime_type);
	} else {
		struct a6o_module **modv;
		GString *s = g_string_new("");
		for (modv = modules; *modv != NULL; modv++)
			g_string_append_printf(s, " %s", (*modv)->name);

		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_DEBUG, "%s: mime-type '%s' ->%s", c->name, mime_type, s->str);

		g_string_free(s, TRUE);
	}

	return modules;
}

void a6o_scan_conf_max_file_size(struct a6o_scan_conf *c, int max_file_size)
{
	c->max_file_size = max_file_size;
}

void a6o_scan_conf_free(struct a6o_scan_conf *scan_conf)
{
	g_array_free(scan_conf->directories_white_list, TRUE);
	g_array_free(scan_conf->mime_types, TRUE);
	g_array_free(scan_conf->modules, TRUE);
	g_hash_table_unref(scan_conf->mime_type_cache);

	free(scan_conf);
}

#ifdef DEBUG
static void mime_type_print(gpointer key, gpointer value, gpointer user_data)
{
	GString *s = (GString *)user_data;
	struct a6o_module **modv;

	g_string_append_printf(s, "    mimetype: %s handled by modules:", (char *)key);

	if (value != NULL)
		for (modv = (struct a6o_module **)value; *modv != NULL; modv++)
			g_string_append_printf(s, " %s", (*modv)->name);
	else
		g_string_append_printf(s, " none");

	g_string_append_printf(s, "\n");
}

const char *a6o_scan_conf_debug(struct a6o_scan_conf *c)
{
	GString *s = g_string_new("");
	const char *ret;

	g_string_append_printf(s, "scan configuration: %s\n", c->name);

	g_hash_table_foreach(c->mime_type_cache, mime_type_print, s);

	ret = s->str;
	g_string_free(s, FALSE);

	return ret;
}
#endif
