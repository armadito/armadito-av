#include "modulePDF.h"

static enum a6o_mod_status modulePDF_init(struct a6o_module *module) {

	// This module doesn't need initialization.
	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "module PDF initialized successfully!");
	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status modulePDF_close(struct a6o_module *module) {

	// This modules doesn't need close instruction.
	return ARMADITO_MOD_OK;
}

static enum a6o_update_status modulePDF_info(struct a6o_module *module, struct a6o_module_info *info){
  
	info->update_date = os_strdup("2016-01-26T09:30:00Z");

	return ARMADITO_UPDATE_OK;
}


static enum a6o_file_status modulePDF_scan(struct a6o_module *module, int fd, const char *path, const char *mime_type, char **pmod_report) {

	enum a6o_file_status status = ARMADITO_CLEAN;
	int ret = 0;


	// Write analysis code here...
	ret = analyzePDF_fd(fd,path);

	if (ret == -1) {
		status = ARMADITO_IERROR;
	}
	else if (ret == -2) {
		status = ARMADITO_UNDECIDED; // Not supported files (encrypted contents or bad header).
	}
	else if (ret < MALICIOUS_COEF) {
		status = ARMADITO_CLEAN;
	}
	else if (ret >= MALICIOUS_COEF) {
		status = ARMADITO_MALWARE;
	}

	return status;
}


struct a6o_module module = {
	.init_fun = modulePDF_init,
	.conf_table= NULL,
	.post_init_fun = NULL,
	.scan_fun = modulePDF_scan,
	.close_fun = modulePDF_close,
	.info_fun = modulePDF_info,
	.name = "modulePDF",
};
