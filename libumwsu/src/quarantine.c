#include <libumwsu/scan.h>
#include <libumwsu/module.h>
#include "dir.h"
#include "modulep.h"
#include "quarantine.h"

#include <stdlib.h>
#include <string.h>

static char *quarantine_dir;
static int quarantine_enabled = 0;

static int quarantine_do(const char *path)
{
  char *newpath;
  int fd;

  newpath = (char *)malloc(strlen(quarantine_dir) + 1 + 6 + 1); /* "QUARANTINE_DIR/XXXXXX" */
  strcpy(newpath, quarantine_dir);
  strcat(newpath, "/XXXXXX");
  if ((fd = mkstemp(newpath)) < 0) {
    perror("mkstemp");
    return -1;
  }

  close(fd);

  if (rename(path, newpath) != 0) {
    perror("rename");
    return -1;
  }

  if (chmod(newpath, 0) != 0) {
    perror("chmod");
    return -1;
  }

  return 0;
}


void quarantine_callback(struct umwsu_report *report, void *callback_data)
{
  switch(report->status) {
  case UMWSU_UNDECIDED:
  case UMWSU_CLEAN:
  case UMWSU_UNKNOWN_FILE_TYPE:
  case UMWSU_EINVAL:
  case UMWSU_IERROR:
  case UMWSU_SUSPICIOUS:
  case UMWSU_WHITE_LISTED:
    return;
  }

  if (!quarantine_do(report->path))
    report->action |= UMWSU_ACTION_QUARANTINE;
}

static enum umwsu_mod_status mod_quarantine_conf(void *mod_data, const char *key, const char *value)
{
  if (!strcmp(key, "quarantine-dir")) {
    fprintf(stderr, "quarantine: got config %s -> %s\n", key, value);
    quarantine_dir = strdup(value);
#if 0
    mkdir_p(quarantine_dir);
#endif
  } else if (!strcmp(key, "enabled")) {
    fprintf(stderr, "quarantine: got config %s -> %s\n", key, value);
    quarantine_enabled = !strcmp(value, "yes") || !strcmp(value, "1") ;
  } 

  return UMWSU_MOD_OK;
}

struct umwsu_module umwsu_mod_quarantine = {
  .init = NULL,
  .conf = &mod_quarantine_conf,
  .scan = NULL,
  .close = NULL,
  .name = "quarantine",
  .mime_types = NULL,
  .data = NULL,
};
