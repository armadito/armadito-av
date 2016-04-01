#include <libarmadito.h>
#include "libarmadito-config.h"

#include "statusp.h"
#include "reportp.h"
#include "os/string.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void a6o_report_init(struct a6o_report *report, int scan_id, const char *path, int progress)
{
	report->scan_id = scan_id;
	if (path != NULL)
		report->path = os_strdup(path);
	else
		report->path = NULL;
	report->progress = progress;
	report->status = ARMADITO_UNDECIDED;
	report->action = ARMADITO_ACTION_NONE;
	report->mod_name = NULL;
	report->mod_report = NULL;
}

void a6o_report_destroy(struct a6o_report *report)
{
	if (report->path != NULL)
		free(report->path);
	if (report->mod_report != NULL)
		free(report->mod_report);
}

void a6o_report_change(struct a6o_report *report, enum a6o_file_status status, const char *mod_name, const char *mod_report)
{
	report->status = status;

	if (mod_name != NULL)
		report->mod_name = (char *)mod_name;

	if (mod_report != NULL) {
		if (report->mod_report != NULL)
			free(report->mod_report);

		report->mod_report = (char *)mod_report;
	}
}
