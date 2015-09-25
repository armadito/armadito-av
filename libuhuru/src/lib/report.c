#include "libuhuru-config.h"

#include "statusp.h"
#include "reportp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void uhuru_report_init(struct uhuru_report *report, const char *path)
{
  report->path = os_strdup(path);
  report->status = UHURU_UNDECIDED;
  report->action = UHURU_ACTION_NONE;
  report->mod_name = NULL;
  report->mod_report = NULL;
}

void uhuru_report_destroy(struct uhuru_report *report)
{
  free(report->path);
  if (report->mod_report != NULL)
    free(report->mod_report);
}

void uhuru_report_change(struct uhuru_report *report, enum uhuru_file_status status, const char *mod_name, const char *mod_report)
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
