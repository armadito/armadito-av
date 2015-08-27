#include <libuhuru/module.h>

#include <assert.h>
#include <clamav.h>
#include <string.h>
#include <stdio.h>

struct clamav_data {
  struct cl_engine *clamav_engine;
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

  module->data = cl_data;

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status clamav_conf_set_dbdir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status clamav_post_init(struct uhuru_module *module)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;
  int ret;
  const char *clamav_db_dir = MODULE_CLAMAV_DBDIR;
#if 0
  /* this if you want to use clamav bases from clamav standard directory */
  const char *clamav_db_dir = cl_retdbdir();
#endif
  unsigned int signature_count = 0;

  if ((ret = cl_load(clamav_db_dir, cl_data->clamav_engine, &signature_count, CL_DB_STDOPT)) != CL_SUCCESS) {
    fprintf(stderr, "ClamAV: error loading databases: %s\n", cl_strerror(ret));
    cl_engine_free(cl_data->clamav_engine);
    cl_data->clamav_engine = NULL;
    return UHURU_MOD_INIT_ERROR;
  }

  fprintf(stderr, "ClamAV database loaded from %s, %d signatures\n", clamav_db_dir, signature_count);

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
  .name = "clamav",
  .size = sizeof(struct clamav_data),
};
