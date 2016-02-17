#include "modulePDF.h"

static enum uhuru_mod_status modulePDF_init(struct uhuru_module *module) {

	// This module doesn't need initialization.
	return UHURU_MOD_OK;
}

static enum uhuru_mod_status modulePDF_close(struct uhuru_module *module) {

	// This modules doesn't need close instruction.
	return UHURU_MOD_OK;
}

static enum uhuru_update_status modulePDF_info(struct uhuru_module *module, struct uhuru_module_info *info){
  
	info->update_date = os_strdup("2016-01-26T09:30:00Z");

	return UHURU_UPDATE_OK;
}


static enum uhuru_file_status modulePDF_scan(struct uhuru_module *module, int fd, const char *path, const char *mime_type, char **pmod_report) {

	enum uhuru_file_status status = UHURU_CLEAN;
	int ret = 0;


	// Write analysis code here...
	ret = analyzePDF_fd(fd,path);

	if (ret == -1) {
		status = UHURU_IERROR;
	}
	else if (ret == -2) {
		status = UHURU_UNDECIDED; // Not supported files (encrypted contents or bad header).
	}
	else if (ret < MALICIOUS_COEF) {
		status = UHURU_CLEAN;
	}
	else if (ret >= MALICIOUS_COEF) {
		status = UHURU_MALWARE;
	}

	return status;
}


struct uhuru_module module = {
	.init_fun = modulePDF_init,
	.conf_table= NULL,
	.post_init_fun = NULL,
	.scan_fun = modulePDF_scan,
	.close_fun = modulePDF_close,
	.info_fun = modulePDF_info,
	.name = "modulePDF",
};
