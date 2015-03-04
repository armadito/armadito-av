#include <libumwsu/module.h>

#include <assert.h>
#include <clamav.h>
#include <string.h>
#include <stdio.h>
#include <stdio.h>

const char *clamav_mime_types[] = {
  "application/x-dosexec",
  "application/pdf",
  "*", /* clamav scans every file if no other module applies */
  NULL,
};

struct clamav_data {
  struct cl_engine *clamav_engine;
};

enum umwsu_file_status clamav_scan(const char *path, const char *mime_type, void *mod_data, char **pmod_report)
{
  struct clamav_data *cl_data = (struct clamav_data *)mod_data;
  const char *virus_name = NULL;
  long unsigned int scanned = 0;
  int cl_scan_status;

  cl_scan_status = cl_scanfile(path, &virus_name, &scanned, cl_data->clamav_engine, CL_SCAN_STDOPT);

  if (cl_scan_status == CL_VIRUS) {
    /* fprintf(stderr, "%s is infected by virus %s!\n", path, virus_name); */
    *pmod_report = strdup(virus_name);

    return UMWSU_MALWARE;
  }

  return UMWSU_CLEAN;
}

enum umwsu_mod_status clamav_init(void **pmod_data)
{
  int ret;
  const char *clamav_db_dir;
  unsigned int signature_count = 0;
  struct clamav_data *cl_data;

  cl_data = (struct clamav_data *)malloc(sizeof(struct clamav_data));
  assert(cl_data != NULL);

  *pmod_data = cl_data;

  if ((ret = cl_init(CL_INIT_DEFAULT)) != CL_SUCCESS) {
    fprintf(stderr, "ClamAV initialization failed: %s\n", cl_strerror(ret));
    return UMWSU_MOD_INIT_ERROR;
  }

  if(!(cl_data->clamav_engine = cl_engine_new())) {
    fprintf(stderr, "ClamAV: can't create new engine\n");
    return UMWSU_MOD_INIT_ERROR;
  }

  clamav_db_dir = cl_retdbdir();

  if ((ret = cl_load(clamav_db_dir, cl_data->clamav_engine, &signature_count, CL_DB_STDOPT)) != CL_SUCCESS) {
    fprintf(stderr, "ClamAV: error loading databases: %s\n", cl_strerror(ret));
    cl_engine_free(cl_data->clamav_engine);
    return UMWSU_MOD_INIT_ERROR;
  }

  fprintf(stderr, "ClamAV database loaded from %s, %d signatures\n", clamav_db_dir, signature_count);

  if ((ret = cl_engine_compile(cl_data->clamav_engine)) != CL_SUCCESS) {
    fprintf(stderr, "ClamAV: engine compilation error: %s\n", cl_strerror(ret));;
    cl_engine_free(cl_data->clamav_engine);
    return UMWSU_MOD_INIT_ERROR;
  }

  fprintf(stderr, "ClamAV is initialized\n");

  return UMWSU_MOD_OK;
}

void clamav_install(void)
{
  fprintf(stderr, "ClamAV module installed\n");
}
