#include <libuhuru/module.h>

#include <assert.h>
#include <clamav.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define _XOPEN_SOURCE
#include <time.h>

struct clamav_data {
  struct cl_engine *clamav_engine;
  const char *db_dir;
  int late_days;
  int critical_days;
};

#define DEFAULT_LATE_DAYS (3)
#define DEFAULT_CRITICAL_DAYS (10)

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

  cl_data->late_days = DEFAULT_LATE_DAYS;
  cl_data->critical_days = DEFAULT_CRITICAL_DAYS;

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

static enum uhuru_mod_status clamav_conf_set_late_days(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;

  cl_data->late_days = atoi(argv[0]);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status clamav_conf_set_critical_days(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;

  cl_data->critical_days = atoi(argv[0]);

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

static void clamav_convert_datetime(const char *clamav_datetime, struct tm *tm_datetime)
{
  char *ret;
  int negative = 0;
  int offset_hour = 0, offset_minute = 0;
  GTimeZone *timezone = NULL;
  GDateTime *datetime, *datetime_utc;
  struct tm tm_tmp;

  memset(&tm_tmp, 0, sizeof(struct tm));
  memset(tm_datetime, 0, sizeof(struct tm));

  /* ClamAV format: 17 Sep 2013 10-57 -0400 */
  ret = strptime(clamav_datetime, "%d%n%b%n%Y%n%H-%M%n", &tm_tmp);
  if (ret == NULL)
    return;

  /* tm_tmp.tm_year += 1900; */

  if (*ret != '\0')
    timezone = g_time_zone_new(ret);

  datetime = g_date_time_new(timezone, tm_tmp.tm_year, tm_tmp.tm_mon, tm_tmp.tm_mday, tm_tmp.tm_hour, tm_tmp.tm_min, tm_tmp.tm_sec);  
  datetime_utc = g_date_time_to_utc(datetime);

  tm_datetime->tm_sec = g_date_time_get_second(datetime_utc);
  tm_datetime->tm_min = g_date_time_get_minute(datetime_utc);
  tm_datetime->tm_hour = g_date_time_get_hour(datetime_utc);
  tm_datetime->tm_mday = g_date_time_get_day_of_month(datetime_utc);
  tm_datetime->tm_mon = g_date_time_get_month(datetime_utc);
  tm_datetime->tm_year = g_date_time_get_year(datetime_utc);

  g_date_time_unref(datetime);
  g_date_time_unref(datetime_utc);
  if (timezone != NULL)
    g_time_zone_unref(timezone);
}

static enum uhuru_update_status clamav_update_status_eval(struct tm *tm_curr, int late_days, int critical_days)
{
  GDateTime *now, *late, *critical, *curr;
  enum uhuru_update_status ret = UHURU_UPDATE_OK;

  now = g_date_time_new_now_utc();
  late = g_date_time_add_days(now, -late_days);
  critical = g_date_time_add_days(now, -critical_days);
  curr = g_date_time_new_utc(tm_curr->tm_year, tm_curr->tm_mon, tm_curr->tm_mday, tm_curr->tm_hour, tm_curr->tm_min, tm_curr->tm_sec);  

  fprintf(stderr, "now       %s\n", g_date_time_format(now, "%FT%H:%M:%SZ"));
  fprintf(stderr, "late      %s\n", g_date_time_format(late, "%FT%H:%M:%SZ"));
  fprintf(stderr, "critical  %s\n", g_date_time_format(critical, "%FT%H:%M:%SZ"));
  fprintf(stderr, "current   %s\n", g_date_time_format(curr, "%FT%H:%M:%SZ"));

  if (g_date_time_compare(curr, critical) < 0)
    ret = UHURU_UPDATE_CRITICAL;
  else if (g_date_time_compare(curr, late) < 0)
    ret = UHURU_UPDATE_LATE;

  g_date_time_unref(now);
  g_date_time_unref(late);
  g_date_time_unref(critical);
  g_date_time_unref(curr);

  return ret;
}

static enum uhuru_update_status clamav_info(struct uhuru_module *module, struct uhuru_module_info *info)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;
  enum uhuru_update_status ret_status = UHURU_UPDATE_OK;
  GString *full_path = g_string_new("");
  GString *version = g_string_new("");
  const char *names[] = { "daily.cld", "bytecode.cld", "main.cvd", };
  int n, i;

  n = sizeof(names) / sizeof(const char *);

  info->base_infos = g_new0(struct uhuru_base_info *, n + 1);

  for (i = 0; i < n; i++) {
    struct cl_cvd *cvd;
    struct uhuru_base_info *base_info;

    g_string_printf(full_path, "%s%s%s", cl_data->db_dir, G_DIR_SEPARATOR_S, names[i]);

    cvd = cl_cvdhead(full_path->str);
    if (cvd == NULL)
      return UHURU_UPDATE_NON_AVAILABLE;
    
    base_info = g_new(struct uhuru_base_info, 1);

    base_info->name = strdup(names[i]);
    g_string_printf(version, "%d", cvd->version);
    base_info->version = strdup(version->str);
    clamav_convert_datetime(cvd->time, &base_info->date);

    base_info->signature_count = cvd->sigs;
    base_info->full_path = strdup(full_path->str);

    /* 
       daily.cld is the base to use to compute update status of module 
       more sophisticated behaviour would be to have late and critical
       offsets for each base, and take the min (or the max, depends) of
       each base status
    */
    if (!strcmp(base_info->name, "daily.cld")) {
      ret_status = clamav_update_status_eval(&base_info->date, cl_data->late_days, cl_data->critical_days);
      info->update_date = base_info->date;
    }

    info->base_infos[i] = base_info;
  }

  g_string_free(full_path, TRUE);
  g_string_free(version, TRUE);
  
  return ret_status;
}

struct uhuru_conf_entry clamav_conf_table[] = {
  { "dbdir", clamav_conf_set_dbdir},
  /* { "db", clamav_conf_set_db}, */
  { "late_days", clamav_conf_set_late_days},
  { "critical_days", clamav_conf_set_critical_days},
  { "db", NULL},
  { NULL, NULL},
};

struct uhuru_module module = {
  .init_fun = clamav_init,
  .conf_table = clamav_conf_table,
  .post_init_fun = clamav_post_init,
  .scan_fun = clamav_scan,
  .close_fun = clamav_close,
  .info_fun = clamav_info,
  .name = "clamav",
  .size = sizeof(struct clamav_data),
};
