#include <libuhuru/module.h>

#include <assert.h>
#include <clamav.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct clamav_data {
  struct cl_engine *clamav_engine;
  const char *db_dir;
};

static enum uhuru_mod_status clamav_init(struct uhuru_module *module)
{
  struct clamav_data *cl_data;
  int ret;

  if ((ret = cl_init(CL_INIT_DEFAULT)) != CL_SUCCESS) {
    fprintf(stderr, "ClamAV initialization failed: %s\n", cl_strerror(ret));
    return UHURU_MOD_INIT_ERROR;
  }

  cl_data = (struct clamav_data *)malloc(sizeof(struct clamav_data));
  assert(cl_data != NULL);

  cl_data->clamav_engine = NULL;

  if(!(cl_data->clamav_engine = cl_engine_new())) {
    fprintf(stderr, "ClamAV: can't create new engine\n");
    return UHURU_MOD_INIT_ERROR;
  }

#if 0
  /* this if you want to use clamav bases from clamav standard directory */
  cl_data->db_dir = cl_retdbdir();
#endif
  cl_data->db_dir = strdup(MODULE_CLAMAV_DBDIR);

  module->data = cl_data;

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status clamav_conf_set_dbdir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;

  if (cl_data->db_dir != NULL)
    free((char *)cl_data->db_dir);

  cl_data->db_dir = strdup(argv[0]);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status clamav_post_init(struct uhuru_module *module)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;
  int ret;
  unsigned int signature_count = 0;

  if ((ret = cl_load(cl_data->db_dir, cl_data->clamav_engine, &signature_count, CL_DB_STDOPT)) != CL_SUCCESS) {
    fprintf(stderr, "ClamAV: error loading databases: %s\n", cl_strerror(ret));
    cl_engine_free(cl_data->clamav_engine);
    cl_data->clamav_engine = NULL;
    return UHURU_MOD_INIT_ERROR;
  }

  fprintf(stderr, "ClamAV database loaded from %s, %d signatures\n", cl_data->db_dir, signature_count);

  if ((ret = cl_engine_compile(cl_data->clamav_engine)) != CL_SUCCESS) {
    fprintf(stderr, "ClamAV: engine compilation error: %s\n", cl_strerror(ret));;
    cl_engine_free(cl_data->clamav_engine);
    cl_data->clamav_engine = NULL;
    return UHURU_MOD_INIT_ERROR;
  }

  fprintf(stderr, "ClamAV is initialized\n");

  return UHURU_MOD_OK;
}

static enum uhuru_file_status clamav_scan(struct uhuru_module *module, const char *path, const char *mime_type, char **pmod_report)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;
  const char *virus_name = NULL;
  long unsigned int scanned = 0;
  int cl_scan_status;

  if (cl_data ->clamav_engine == NULL)
    return UHURU_IERROR;

  cl_scan_status = cl_scanfile(path, &virus_name, &scanned, cl_data->clamav_engine, CL_SCAN_STDOPT);

  if (cl_scan_status == CL_VIRUS) {
    *pmod_report = strdup(virus_name);

    return UHURU_MALWARE;
  }

  return UHURU_CLEAN;
}

static enum uhuru_mod_status clamav_close(struct uhuru_module *module)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;
  int ret;

  if ((ret = cl_engine_free(cl_data->clamav_engine)) != CL_SUCCESS) {
    fprintf(stderr, "ClamAV: can't free engine\n");
    return UHURU_MOD_CLOSE_ERROR;
  }

  cl_data->clamav_engine = NULL;

  return UHURU_MOD_OK;
}

static enum uhuru_update_status clamav_update_check(struct uhuru_module *module)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;
  GString *full_path = g_string_new("");
  GString *version = g_string_new("");
  const char *names[] = { "bytecode.cld", "daily.cld", "main.cvd", };
  int n, i;

  n = sizeof(names) / sizeof(const char *);

  module->base_infos = g_new0(struct uhuru_base_info *, n + 1);

  for (i = 0; i < n; i++) {
    struct cl_cvd *cvd;
    struct uhuru_base_info *info;

    g_string_printf(full_path, "%s%s%s", cl_data->db_dir, G_DIR_SEPARATOR_S, names[i]);

    cvd = cl_cvdhead(full_path->str);
    if (cvd == NULL)
      return UHURU_UPDATE_NON_AVAILABLE;
    
    info = g_new(struct uhuru_base_info, 1);

    info->name = strdup(names[i]);
    info->date = strdup(cvd->time);
    g_string_printf(version, "%d", cvd->version);
    info->version = strdup(version->str);
    info->signature_count = cvd->sigs;
    info->full_path = strdup(full_path->str);

    module->base_infos[i] = info;
  }

  g_string_free(full_path, TRUE);
  g_string_free(version, TRUE);
  
  return UHURU_UPDATE_OK;
}

struct uhuru_conf_entry clamav_conf_table[] = {
  { "dbdir", clamav_conf_set_dbdir},
  /* { "db", clamav_conf_set_db}, */
  { "db", NULL},
  { NULL, NULL},
};

struct uhuru_module module = {
  .init_fun = clamav_init,
  .conf_table = clamav_conf_table,
  .post_init_fun = clamav_post_init,
  .scan_fun = clamav_scan,
  .close_fun = clamav_close,
  .update_check_fun = clamav_update_check,
  .name = "clamav",
  .size = sizeof(struct clamav_data),
};
