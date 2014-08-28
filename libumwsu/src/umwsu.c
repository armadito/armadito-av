#include <libumwsu/module.h>
#include <libumwsu/scan.h>
#include "dir.h"
#include "modulep.h"

#include <assert.h>
#include <glib.h>
#include <magic.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct umwsu {
  int verbosity;
  GHashTable *mime_types_table;
  GPtrArray *modules;
  magic_t magic;
};

static struct umwsu *umwsu_new(void)
{
  struct umwsu *u = (struct umwsu *)malloc(sizeof(struct umwsu));

  assert(u != NULL);

  u->verbosity = 0;

  u->magic = magic_open(MAGIC_MIME_TYPE);
  magic_load(u->magic, NULL);

  u->mime_types_table = g_hash_table_new(g_str_hash, g_str_equal);
  u->modules = g_ptr_array_new();

  return u;
}

void umwsu_set_verbose(struct umwsu *u, int verbosity)
{
  u->verbosity = verbosity;
}

static void umwsu_add_module(struct umwsu *u, struct umwsu_module *mod)
{
  g_ptr_array_add(u->modules, mod);
}

static void load_entry(const char *full_path, void *data)
{
  struct umwsu *u = (struct umwsu *)data;
  const char *t = magic_file(u->magic, full_path);
  struct umwsu_module *mod;

  if (strcmp("application/x-sharedlib", t))
    return;

  if (u->verbosity >= 1)
    printf("UMWSU: loading module object: %s\n", full_path);

  mod = module_new(full_path);

  umwsu_add_module(u, mod);
}

static int umwsu_module_load_directory(struct umwsu *u, const char *directory)
{
  return dir_map(directory, 0, load_entry, u);
}

static void umwsu_add_mime_type(struct umwsu *u, const char *mime_type, struct umwsu_module *mod)
{
  GPtrArray *mod_array;

  mod_array = (GPtrArray *)g_hash_table_lookup(u->mime_types_table, mime_type);

  if (mod_array == NULL) {
    mod_array = g_ptr_array_new();

    g_hash_table_insert(u->mime_types_table, (gpointer)mime_type, mod_array);
  }

  g_ptr_array_add(mod_array, mod);
}

static void uwm_module_init_all(struct umwsu *u)
{
  int i;

  for (i = 0; i < u->modules->len; i++) {
    struct umwsu_module *mod = (struct umwsu_module *)g_ptr_array_index(u->modules, i);

    if (mod->init != NULL)
      (*mod->init)(&mod->data);

    if (mod->mime_types != NULL) {
      const char **p;

      for(p = mod->mime_types; *p != NULL; p++)
	umwsu_add_mime_type(u, *p, mod);
    }
  }
}

static void mod_print(gpointer data, gpointer user_data)
{
  module_print((struct umwsu_module *)data, stderr);
}

static void uwm_module_print_all(struct umwsu *u)
{
  g_ptr_array_foreach(u->modules, mod_print, NULL);
}

struct umwsu *umwsu_open(void)
{
  struct umwsu *u = umwsu_new();

  umwsu_module_load_directory(u, LIBUMWSU_MODULES_PATH);

  /* module_conf_all(); */

  uwm_module_init_all(u);

  /* uwm_module_print_all(u); */

  return u;
}

static void mod_print_name(gpointer data, gpointer user_data)
{
  struct umwsu_module *mod = (struct umwsu_module *)data;
  printf("%s ", mod->name);
}

void print_mime_type_entry(gpointer key, gpointer value, gpointer user_data)
{
  printf("UMWSU: MIME type: %s handled by module: ", (char *)key);
  g_ptr_array_foreach((GPtrArray *)value, mod_print_name, NULL);
  printf("\n");
}

void umwsu_print(struct umwsu *u)
{
  printf("UMWSU: modules loaded: ");
  g_ptr_array_foreach(u->modules, mod_print_name, NULL);
  printf("\n");

  g_hash_table_foreach(u->mime_types_table, print_mime_type_entry, NULL);
}

void umwsu_close(struct umwsu *u)
{
  magic_close(u->magic);
}

static int umwsu_status_cmp(enum umwsu_status s1, enum umwsu_status s2)
{
  if (s1 == s2)
    return 0;

  switch(s1) {
  case UMWSU_CLEAN:
    return -1;
  case UMWSU_IERROR:
    return (s2 == UMWSU_CLEAN) ? 1 : -1;
  case UMWSU_SUSPICIOUS:
    return (s2 == UMWSU_CLEAN || s2 == UMWSU_IERROR) ? 1 : -1;
  case UMWSU_MALWARE:
    return 1;
  }

  assert(1 == 0);

  return 0;
}


enum umwsu_status umwsu_scan_file(struct umwsu *u, const char *path, struct umwsu_report *report)
{
  const char *mime_type = magic_file(u->magic, path);
  GPtrArray *mod_array;
  enum umwsu_status current_status = UMWSU_CLEAN;

  report->status = UMWSU_CLEAN;
  report->path = strdup(path);
  report->mod_name = NULL;
  report->mod_report = NULL;

  mod_array = (GPtrArray *)g_hash_table_lookup(u->mime_types_table, mime_type);

  if (mod_array == NULL) {
    current_status = UMWSU_UNKNOWN_FILE_TYPE;

    report->status = current_status;
  } else {
    int i;

    for (i = 0; i < mod_array->len; i++) {
      struct umwsu_module *mod = (struct umwsu_module *)g_ptr_array_index(mod_array, i);
      enum umwsu_status mod_status;
      char *mod_report = NULL;

      if (u->verbosity >= 2)
	printf("UMWSU: module %s: scanning %s\n", mod->name, path);

      mod_status = (*mod->scan)(path, mod->data, &mod_report);

      if (umwsu_status_cmp(current_status, mod_status) < 0) {
	current_status = mod_status;

	report->status = mod_status;
	report->mod_name = (char *)mod->name;
	if (report->mod_report != NULL)
	  free(report->mod_report);
	report->mod_report = mod_report;
      } else if (mod_report != NULL)
	free(mod_report);

      if (current_status == UMWSU_MALWARE)
	break;
    }
  }

  if (u->verbosity >= 3)
    printf("%s: %s\n", path, umwsu_status_str(current_status));

  return current_status;
}

static void scan_entry(const char *full_path, void *data)
{
  struct umwsu *u = (struct umwsu *)data;
  struct umwsu_report report;

  umwsu_scan_file(u, full_path, &report);
  umwsu_report_print(&report, stdout);
}

enum umwsu_status umwsu_scan_dir(struct umwsu *u, const char *path, int recurse)
{
  dir_map(path, recurse, scan_entry, u);

  return UMWSU_CLEAN;
}

const char *umwsu_status_str(enum umwsu_status status)
{
  switch(status) {
#define M(S) case S: return #S
    M(UMWSU_CLEAN);
    M(UMWSU_SUSPICIOUS);
    M(UMWSU_MALWARE);
    M(UMWSU_EINVAL);
    M(UMWSU_IERROR);
    M(UMWSU_UNKNOWN_FILE_TYPE);
  }

  return "UNKNOWN STATUS";
}

const char *umwsu_status_pretty_str(enum umwsu_status status)
{
  switch(status) {
  case UMWSU_CLEAN:
    return "clean";
  case UMWSU_SUSPICIOUS:
    return "suspicious";
  case UMWSU_MALWARE:
    return "malware";
  case UMWSU_EINVAL:
    return "invalid argument";
  case UMWSU_IERROR:
    return "internal error";
  case UMWSU_UNKNOWN_FILE_TYPE:
    return "ignored";
  }

  return "inconnu";
}

void umwsu_report_print(struct umwsu_report *report, FILE *out)
{
  fprintf(out, "%s: %s", report->path, umwsu_status_pretty_str(report->status));
  if (report->status != UMWSU_CLEAN && report->status != UMWSU_UNKNOWN_FILE_TYPE)
    fprintf(out, " [%s - %s]", report->mod_name, report->mod_report);
  fprintf(out, "\n");
}
