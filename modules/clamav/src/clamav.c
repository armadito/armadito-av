#include <libarmadito.h>

#include <assert.h>
#include <clamav.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _XOPEN_SOURCE
#include <time.h>

#include "os/osdeps.h"

struct clamav_data {
	struct cl_engine *clamav_engine;
	const char *db_dir;
	const char *tmp_dir;
	int late_days;
	int critical_days;
};

#define DEFAULT_LATE_DAYS (3)
#define DEFAULT_CRITICAL_DAYS (10)

static enum a6o_mod_status clamav_init(struct a6o_module *module)
{
	struct clamav_data *cl_data;
	int ret;
	const char *bases_location;
	char *db_dir;
	size_t len;

	if ((ret = cl_init(CL_INIT_DEFAULT)) != CL_SUCCESS) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "ClamAV initialization failed: %s", cl_strerror(ret));
		return ARMADITO_MOD_INIT_ERROR;
	}

	cl_data = malloc(sizeof(struct clamav_data));

	cl_data->clamav_engine = cl_engine_new();
	if(!cl_data->clamav_engine) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "ClamAV: can't create new engine");
		return ARMADITO_MOD_INIT_ERROR;
	}

	bases_location = a6o_std_path(BASES_LOCATION);
	len = strlen(bases_location) + 1 + strlen("clamav") + 1;
	db_dir = malloc(len);
	sprintf(db_dir, "%s%cclamav", bases_location, a6o_path_sep());

	cl_data->db_dir = db_dir;
	cl_data->tmp_dir = NULL;
	cl_data->late_days = DEFAULT_LATE_DAYS;
	cl_data->critical_days = DEFAULT_CRITICAL_DAYS;

	module->data = cl_data;

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status clamav_conf_set_dbdir(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct clamav_data *cl_data = (struct clamav_data *)module->data;

	if (cl_data->db_dir != NULL)
		free((char *)cl_data->db_dir);

	cl_data->db_dir = os_strdup(a6o_conf_value_get_string(value));

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status clamav_conf_set_tmpdir(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct clamav_data *cl_data = (struct clamav_data *)module->data;

	if (cl_data->tmp_dir != NULL)
		free((char *)cl_data->tmp_dir);

	cl_data->tmp_dir = os_strdup(a6o_conf_value_get_string(value));

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status clamav_conf_set_late_days(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct clamav_data *cl_data = (struct clamav_data *)module->data;

	cl_data->late_days = a6o_conf_value_get_int(value);

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status clamav_conf_set_critical_days(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct clamav_data *cl_data = (struct clamav_data *)module->data;

	cl_data->critical_days = a6o_conf_value_get_int(value);

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status clamav_post_init(struct a6o_module *module)
{
	struct clamav_data *cl_data = (struct clamav_data *)module->data;
	int ret;
	unsigned int signature_count = 0;

	if (cl_data->tmp_dir != NULL) {
		if ((ret = cl_engine_set_str(cl_data->clamav_engine, CL_ENGINE_TMPDIR, cl_data->tmp_dir)) != CL_SUCCESS) {
			a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "ClamAV: error setting temporary directory: %s", cl_strerror(ret));
			cl_engine_free(cl_data->clamav_engine);
			cl_data->clamav_engine = NULL;
			return ARMADITO_MOD_INIT_ERROR;
		}
	}

	if ((ret = cl_load(cl_data->db_dir, cl_data->clamav_engine, &signature_count, CL_DB_STDOPT)) != CL_SUCCESS) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "ClamAV: error loading databases: %s", cl_strerror(ret));
		cl_engine_free(cl_data->clamav_engine);
		cl_data->clamav_engine = NULL;
		return ARMADITO_MOD_INIT_ERROR;
	}

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "ClamAV database loaded from %s, %d signatures", cl_data->db_dir, signature_count);

	if ((ret = cl_engine_compile(cl_data->clamav_engine)) != CL_SUCCESS) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "ClamAV: engine compilation error: %s", cl_strerror(ret));;
		cl_engine_free(cl_data->clamav_engine);
		cl_data->clamav_engine = NULL;
		return ARMADITO_MOD_INIT_ERROR;
	}

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "ClamAV is initialized");

	return ARMADITO_MOD_OK;
}

static enum a6o_file_status clamav_scan(struct a6o_module *module, int fd, const char *path, const char *mime_type, char **pmod_report)
{
	struct clamav_data *cl_data = (struct clamav_data *)module->data;
	const char *virus_name = NULL;
	long unsigned int scanned = 0;
	int cl_scan_status;

	if (cl_data ->clamav_engine == NULL)
		return ARMADITO_IERROR;

	cl_scan_status = cl_scandesc(fd, &virus_name, &scanned, cl_data->clamav_engine, CL_SCAN_STDOPT);

	if (cl_scan_status == CL_VIRUS) {
		*pmod_report = os_strdup(virus_name);

		return ARMADITO_MALWARE;
	}

	return ARMADITO_CLEAN;
}

static enum a6o_mod_status clamav_close(struct a6o_module *module)
{
	struct clamav_data *cl_data = (struct clamav_data *)module->data;
	int ret;

	if ((ret = cl_engine_free(cl_data->clamav_engine)) != CL_SUCCESS) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "ClamAV: can't free engine");
		return ARMADITO_MOD_CLOSE_ERROR;
	}

	cl_data->clamav_engine = NULL;

	// Ulrich add
	cl_cleanup_crypto();

	return ARMADITO_MOD_OK;
}

/* under re-implementation */
static enum a6o_update_status clamav_info(struct a6o_module *module, struct a6o_module_info *info)
{
	return ARMADITO_UPDATE_OK;
}

static struct a6o_conf_entry clamav_conf_table[] = {
	{ "critical_days", CONF_TYPE_INT, clamav_conf_set_critical_days},
	{ "late_days", CONF_TYPE_INT, clamav_conf_set_late_days},
	{ "dbdir", CONF_TYPE_STRING, clamav_conf_set_dbdir},
	{ "tmpdir", CONF_TYPE_STRING, clamav_conf_set_tmpdir},
	{ NULL, CONF_TYPE_VOID, NULL},
};

static const char *clamav_mime_types[] = { "*", NULL, };

struct a6o_module module = {
	.init_fun = clamav_init,
	.conf_table = clamav_conf_table,
	.post_init_fun = clamav_post_init,
	.scan_fun = clamav_scan,
	.close_fun = clamav_close,
	.info_fun = clamav_info,
	.supported_mime_types = clamav_mime_types,
	.name = "clamav",
	.size = sizeof(struct clamav_data),
};
