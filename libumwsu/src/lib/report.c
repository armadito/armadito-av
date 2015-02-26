#include "statusp.h"
#include "reportp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void umwsu_report_print(struct umwsu_report *report, FILE *out)
{
  fprintf(out, "%s: %s", report->path, umwsu_status_pretty_str(report->status));
  if (report->status != UMWSU_UNDECIDED && report->status != UMWSU_CLEAN && report->status != UMWSU_UNKNOWN_FILE_TYPE)
    fprintf(out, " [%s - %s]", report->mod_name, report->mod_report);
  if (report->action != UMWSU_ACTION_NONE)
    fprintf(out, " (action %s)", umwsu_action_pretty_str(report->action));
  fprintf(out, "\n");
}

void umwsu_report_init(struct umwsu_report *report, const char *path)
{
  report->path = strdup(path);
  report->status = UMWSU_UNDECIDED;
  report->action = UMWSU_ACTION_NONE;
  report->mod_name = NULL;
  report->mod_report = NULL;
}

void umwsu_report_destroy(struct umwsu_report *report)
{
  free(report->path);
  if (report->mod_report != NULL)
    free(report->mod_report);
}

void umwsu_report_change(struct umwsu_report *report, enum umwsu_status status, const char *mod_name, const char *mod_report)
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
