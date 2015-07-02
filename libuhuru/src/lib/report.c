#include "statusp.h"
#include "reportp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void uhuru_report_print(struct uhuru_report *report, FILE *out)
{
  fprintf(out, "%s: %s", report->path, uhuru_file_status_pretty_str(report->status));
  if (report->status != UHURU_UNDECIDED && report->status != UHURU_CLEAN && report->status != UHURU_UNKNOWN_FILE_TYPE)
    fprintf(out, " [%s - %s]", report->mod_name, report->mod_report);
  if (report->action != UHURU_ACTION_NONE)
    fprintf(out, " (action %s)", uhuru_action_pretty_str(report->action));
  fprintf(out, "\n");
}

void uhuru_report_init(struct uhuru_report *report, const char *path)
{
  report->path = strdup(path);
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
