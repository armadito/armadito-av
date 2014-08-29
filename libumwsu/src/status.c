#include "statusp.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int umwsu_status_cmp(enum umwsu_status s1, enum umwsu_status s2)
{
  if (s1 == s2)
    return 0;

  switch(s1) {
  case UMWSU_CLEAN:
    return -1;
  case UMWSU_IERROR:
    return (s2 == UMWSU_CLEAN) ? 1 : -1;
  case UMWSU_SUSPICIOUS:
    return (s2 == UMWSU_CLEAN || s2 == UMWSU_IERROR) ? 1 : -1;
  case UMWSU_MALWARE:
    return 1;
  }

  assert(1 == 0);

  return 0;
}

const char *umwsu_status_str(enum umwsu_status status)
{
  switch(status) {
#define M(S) case S: return #S
    M(UMWSU_CLEAN);
    M(UMWSU_SUSPICIOUS);
    M(UMWSU_MALWARE);
    M(UMWSU_EINVAL);
    M(UMWSU_IERROR);
    M(UMWSU_UNKNOWN_FILE_TYPE);
  }

  return "UNKNOWN STATUS";
}

const char *umwsu_status_pretty_str(enum umwsu_status status)
{
  switch(status) {
  case UMWSU_CLEAN:
    return "clean";
  case UMWSU_SUSPICIOUS:
    return "suspicious";
  case UMWSU_MALWARE:
    return "malware";
  case UMWSU_EINVAL:
    return "invalid argument";
  case UMWSU_IERROR:
    return "internal error";
  case UMWSU_UNKNOWN_FILE_TYPE:
    return "ignored";
  }

  return "inconnu";
}

void umwsu_report_print(struct umwsu_report *report, FILE *out)
{
  fprintf(out, "%s: %s", report->path, umwsu_status_pretty_str(report->status));
  if (report->status != UMWSU_CLEAN && report->status != UMWSU_UNKNOWN_FILE_TYPE)
    fprintf(out, " [%s - %s]", report->mod_name, report->mod_report);
  fprintf(out, "\n");
}

void umwsu_report_init(struct umwsu_report *report, const char *path)
{
  report->status = UMWSU_CLEAN;
  report->path = strdup(path);
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
