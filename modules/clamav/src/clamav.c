#include <libuhuru/core.h>

#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define _XOPEN_SOURCE
#include <time.h>
#include "os/osdeps.h"

#ifdef WIN32
char * GetDBDirectory( ) {

	char * dirpath = NULL;
	char filepath[MAX_PATH];
	char * ptr = NULL;
	int len = 0;
	int size = 0;

	if (!GetModuleFileNameA(NULL, &filepath, MAX_PATH)) {		
		g_log(NULL, G_LOG_LEVEL_WARNING, "[-] Error :: GetBinaryDirectory!GetModuleFileName() failed :: %d\n",GetLastError());
		return NULL;
	}	

	// get the file name from the complete file path
	ptr = strrchr(filepath,'\\');
	if (ptr == NULL) {		
		g_log(NULL, G_LOG_LEVEL_WARNING, "[-] Error :: GetBinaryDirectory!strrchr() failed :: backslash not found in the path.\n");
		return NULL;
	}
	
	// calc the dir buffer length.
	size = (int)(ptr - filepath);
	len = size + strnlen_s(MODULE_CLAMAV_DBDIR, MAX_PATH) + 1;
	//printf("[+] Debug :: GetBinaryDirectory :: ptr=%d :: filepath =%d :: len = %d :: strlen = %d\n",ptr,filepath,len,strlen(filepath));

	dirpath = (char*)(calloc(len+1,sizeof(char)));
	dirpath[len] = '\0';

	memcpy_s(dirpath, len, filepath, size);
	dirpath[size] = '\\';
	memcpy_s(dirpath+size+1,len,MODULE_CLAMAV_DBDIR,strnlen_s(MODULE_CLAMAV_DBDIR, _MAX_PATH));


	//g_log(NULL, G_LOG_LEVEL_WARNING, "[+] Debug :: GetBinaryDirectory :: dirpath = %s\n",dirpath);

	return dirpath;
}
#endif

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
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "ClamAV initialization failed: %s", cl_strerror(ret));
    return UHURU_MOD_INIT_ERROR;
  }

  cl_data = (struct clamav_data *)malloc(sizeof(struct clamav_data));
  assert(cl_data != NULL);

  cl_data->clamav_engine = NULL;

  if(!(cl_data->clamav_engine = cl_engine_new())) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "ClamAV: can't create new engine");
    return UHURU_MOD_INIT_ERROR;
  }

#if 0
  /* this if you want to use clamav bases from clamav standard directory */
  cl_data->db_dir = cl_retdbdir();
#endif

#ifdef WIN32
 
  cl_data->db_dir = GetDBDirectory();
  
#else
  cl_data->db_dir = os_strdup(MODULE_CLAMAV_DBDIR);
#endif

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

  cl_data->db_dir = os_strdup(argv[0]);

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
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "ClamAV: error loading databases: %s", cl_strerror(ret));
    cl_engine_free(cl_data->clamav_engine);
    cl_data->clamav_engine = NULL;
    return UHURU_MOD_INIT_ERROR;
  }

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, "ClamAV database loaded from %s, %d signatures", cl_data->db_dir, signature_count);

  if ((ret = cl_engine_compile(cl_data->clamav_engine)) != CL_SUCCESS) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "ClamAV: engine compilation error: %s", cl_strerror(ret));;
    cl_engine_free(cl_data->clamav_engine);
    cl_data->clamav_engine = NULL;
    return UHURU_MOD_INIT_ERROR;
  }

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, "ClamAV is initialized");

  return UHURU_MOD_OK;
}

static enum uhuru_file_status clamav_scan(struct uhuru_module *module, int fd, const char *path, const char *mime_type, char **pmod_report)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;
  const char *virus_name = NULL;
  long unsigned int scanned = 0;
  int cl_scan_status;

  if (cl_data ->clamav_engine == NULL)
    return UHURU_IERROR;

  cl_scan_status = cl_scandesc(fd, &virus_name, &scanned, cl_data->clamav_engine, CL_SCAN_STDOPT);
  /* cl_scan_status = cl_scanfile(path, &virus_name, &scanned, cl_data->clamav_engine, CL_SCAN_STDOPT); */

  if (cl_scan_status == CL_VIRUS) {
    *pmod_report = os_strdup(virus_name);
	
    return UHURU_MALWARE;
  }

  return UHURU_CLEAN;
}

static enum uhuru_mod_status clamav_close(struct uhuru_module *module)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;
  int ret;

  
  if ((ret = cl_engine_free(cl_data->clamav_engine)) != CL_SUCCESS) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "ClamAV: can't free engine");
    return UHURU_MOD_CLOSE_ERROR;
  }

  cl_data->clamav_engine = NULL;

  // Ulrich add
  cl_cleanup_crypto(); 

  return UHURU_MOD_OK;
}

static int get_month(const char *month)
{
  static const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  const char *ret;

  if ((ret = strstr(months, month)) == NULL)
    return -1;

  return (ret - months) / 3 + 1;
}

static GDateTime *clamav_convert_datetime(const char *clamav_datetime)
{
  int day, year, hour, minute;
  char s_month[4], s_timezone[6];
  GDateTime *datetime, *datetime_utc;
  GTimeZone *timezone = NULL;

  /* ClamAV format: 17 Sep 2013 10-57 -0400 */
  os_sscanf(clamav_datetime, "%d %3s %d %2d-%2d %5s", &day, s_month, &year, &hour, &minute, s_timezone);

  timezone = g_time_zone_new(s_timezone);

  datetime = g_date_time_new(timezone, year, get_month(s_month), day, hour, minute, 0);  

  datetime_utc = g_date_time_to_utc(datetime);

  g_date_time_unref(datetime);
  if (timezone != NULL)
    g_time_zone_unref(timezone);

  return datetime_utc;
}

static enum uhuru_update_status clamav_update_status_eval(GDateTime *curr, int late_days, int critical_days)
{
  GDateTime *now, *late, *critical;
  enum uhuru_update_status ret = UHURU_UPDATE_OK;

  now = g_date_time_new_now_utc();
  late = g_date_time_add_days(now, -late_days);
  critical = g_date_time_add_days(now, -critical_days);

  if (g_date_time_compare(curr, critical) < 0)
    ret = UHURU_UPDATE_CRITICAL;
  else if (g_date_time_compare(curr, late) < 0)
    ret = UHURU_UPDATE_LATE;

  g_date_time_unref(now);
  g_date_time_unref(late);
  g_date_time_unref(critical);

  return ret;
}

static enum uhuru_update_status clamav_info(struct uhuru_module *module, struct uhuru_module_info *info)
{
  struct clamav_data *cl_data = (struct clamav_data *)module->data;
  enum uhuru_update_status ret_status = UHURU_UPDATE_OK;
  GString *full_path = g_string_new("");
  GString *version = g_string_new("");
  const char *names[] = { "daily.cld", "bytecode.cld", "main.cvd", };
  GDateTime *base_datetime;
  int n, i;

  n = sizeof(names) / sizeof(const char *);

  info->base_infos = g_new0(struct uhuru_base_info *, n + 1);

  for (i = 0; i < n; i++) {
    struct cl_cvd *cvd;
    struct uhuru_base_info *base_info;
    char *s_base_datetime;

    g_string_printf(full_path, "%s%s%s", cl_data->db_dir, G_DIR_SEPARATOR_S, names[i]);

    cvd = cl_cvdhead(full_path->str);
    if (cvd == NULL)
      return UHURU_UPDATE_NON_AVAILABLE;
    
    base_info = g_new(struct uhuru_base_info, 1);

    base_info->name = os_strdup(names[i]);

    g_string_printf(version, "%d", cvd->version);
    base_info->version = os_strdup(version->str);

    base_datetime = clamav_convert_datetime(cvd->time);
    s_base_datetime = g_date_time_format(base_datetime, "%FT%H:%M:%SZ");
    base_info->date = os_strdup(s_base_datetime);
    g_free(s_base_datetime);

    base_info->signature_count = cvd->sigs;
    base_info->full_path = os_strdup(full_path->str);

    /* 
       daily.cld is the base to use to compute update status of module 
       more sophisticated behaviour would be to have late and critical
       offsets for each base, and take the min (or the max, depends) of
       each base status
    */
    if (!strcmp(base_info->name, "daily.cld")) {
      ret_status = clamav_update_status_eval(base_datetime, cl_data->late_days, cl_data->critical_days);
      info->update_date = os_strdup(base_info->date);
    }

    info->base_infos[i] = base_info;

    g_date_time_unref(base_datetime);
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
