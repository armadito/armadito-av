/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito module PDF.

Armadito module PDF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito module PDF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito module PDF.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "modulePDF.h"

struct modulePDF_data {
	const char *tmp_dir;
	int late_days;
	int critical_days;
};

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
  
	time_t ts = 0;		
	struct tm timeptr = {0, 30, 8, 1, 2, 116}; // 01/03/2016 9:30
	info->update_date = os_strdup("2016-01-26T09:30:00Z");

	ts=mktime(&timeptr);
	info->timestamp = ts;

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
		
		*pmod_report = os_strdup("ModulePDF!SuspiciousPDF");
	}

	return status;
}


static const char *modulePDF_mime_types[] = {
	"application/pdf",
	NULL,
};

struct a6o_module module = {
	.init_fun = modulePDF_init,
	.conf_table = NULL,
	.post_init_fun = NULL,
	.scan_fun = modulePDF_scan,
	.close_fun = modulePDF_close,
	.info_fun = modulePDF_info,
	.supported_mime_types = modulePDF_mime_types,
	.name = "modulePDF",
	.size = sizeof(struct modulePDF_data),
};
