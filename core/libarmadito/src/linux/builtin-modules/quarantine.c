/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#include <libarmadito.h>
#include "os/dir.h"
#include "quarantine.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct quarantine_data {
	char *quarantine_dir;
	int enable;
};

static enum a6o_mod_status quarantine_init(struct a6o_module *module)
{
	struct quarantine_data *qu_data = g_new(struct quarantine_data, 1);

	qu_data->quarantine_dir = NULL;
	qu_data->enable = 0;

	module->data = qu_data;

	return ARMADITO_MOD_OK;
}

static int quarantine_do(struct quarantine_data *qu_data, const char *path)
{
	char *newpath;
	int fd;
	struct stat stat_buf;
	mode_t mode = -1;
	uid_t uid = -1;
	gid_t gid = -1;
	FILE *info;
	int ret = 0;

	newpath = (char *)malloc(strlen(qu_data->quarantine_dir) + 1 + 6 + 5 + 1); /* "QUARANTINE_DIR/XXXXXX[.info]" */
	strcpy(newpath, qu_data->quarantine_dir);
	strcat(newpath, "/XXXXXX");

	/* coverity[secure_temp] */
	if ((fd = mkstemp(newpath)) < 0) {
		perror("mkstemp");
		ret = -1;
		goto get_out;
	}

	close(fd);

	if(!stat(path, &stat_buf)) {
		mode = stat_buf.st_mode;
		uid = stat_buf.st_uid;
		gid = stat_buf.st_gid;
	} else {
		ret = -2;
		goto get_out;
	}

	if (rename(path, newpath) != 0) {
		perror("rename");
		ret = -1;
		goto get_out;
	}

	if (chmod(newpath, 0) != 0) {
		perror("chmod");
		ret = -2;
		/* continue either */
	}

	strcat(newpath, ".info");
	info = fopen(newpath, "w");
	if (info == NULL) {
		ret = -2;
		goto get_out;
	}

	fprintf(info, "path: %s\n", path);
	fprintf(info, "mode: 0%o\n", mode);
	fprintf(info, "uid: %d\n", uid);
	fprintf(info, "gid: %d\n", gid);

	fclose(info);

get_out:
	free(newpath);
	return ret;
}

void quarantine_callback(struct a6o_report *report, void *callback_data)
{
	struct quarantine_data *qu_data = (struct quarantine_data *)callback_data;

	if (!qu_data->enable)
		return;

	switch(report->status) {
	case ARMADITO_UNDECIDED:
	case ARMADITO_CLEAN:
	case ARMADITO_UNKNOWN_FILE_TYPE:
	case ARMADITO_EINVAL:
	case ARMADITO_IERROR:
	case ARMADITO_SUSPICIOUS:
	case ARMADITO_WHITE_LISTED:
		return;
	}

	if (quarantine_do(qu_data, report->path) != -1)
		report->action |= ARMADITO_ACTION_QUARANTINE;
}

static enum a6o_mod_status quarantine_conf_quarantine_dir(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct quarantine_data *qu_data = (struct quarantine_data *)module->data;

	qu_data->quarantine_dir = strdup(a6o_conf_value_get_string(value));

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status quarantine_conf_enable(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct quarantine_data *qu_data = (struct quarantine_data *)module->data;

	qu_data->enable = a6o_conf_value_get_int(value);

	return ARMADITO_MOD_OK;
}

static struct a6o_conf_entry quarantine_conf_table[] = {
	{
		.key = "quarantine-dir",
		.type = CONF_TYPE_STRING,
		.conf_fun = quarantine_conf_quarantine_dir,
	},
	{
		.key = "enable",
		.type = CONF_TYPE_INT,
		.conf_fun = quarantine_conf_enable,
	},
	{
		.key = NULL,
		.type = 0,
		.conf_fun = NULL,
	},
};

struct a6o_module quarantine_module = {
	.init_fun = quarantine_init,
	.conf_table = quarantine_conf_table,
	.post_init_fun = NULL,
	.scan_fun = NULL,
	.close_fun = NULL,
	.supported_mime_types = NULL,
	.name = "quarantine",
	.size = sizeof(struct quarantine_data),
};
