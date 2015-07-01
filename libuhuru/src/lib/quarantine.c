#include <libumwsu/scan.h>
#include <libumwsu/module.h>
#include "dir.h"
#include "modulep.h"
#include "quarantine.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static char *quarantine_dir;
static int quarantine_enabled = 0;

static int quarantine_do(const char *path)
{
  char *newpath;
  int fd;
  struct stat stat_buf;
  mode_t mode = -1;
  uid_t uid = -1;
  gid_t gid = -1;
  FILE *info;
  int ret = 0;

  newpath = (char *)malloc(strlen(quarantine_dir) + 1 + 6 + 5 + 1); /* "QUARANTINE_DIR/XXXXXX[.info]" */
  strcpy(newpath, quarantine_dir);
  strcat(newpath, "/XXXXXX");
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
  return ret;
}


void quarantine_callback(struct umwsu_report *report, void *callback_data)
{
  if (!quarantine_enabled)
    return;

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

  if (quarantine_do(report->path) != -1)
    report->action |= UMWSU_ACTION_QUARANTINE;
}

static enum umwsu_mod_status mod_quarantine_conf_set(void *mod_data, const char *key, const char *value)
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

static char *mod_quarantine_conf_get(void *mod_data, const char *key)
{
  if (!strcmp(key, "quarantine-dir")) {
    return quarantine_dir;
  } else if (!strcmp(key, "enabled")) {
    return (quarantine_enabled) ? "1" : "0";
  } 

  return NULL;
}

struct umwsu_module umwsu_mod_quarantine = {
  .init = NULL,
  .conf_set = &mod_quarantine_conf_set,
  .conf_get = &mod_quarantine_conf_get,
  .scan = NULL,
  .close = NULL,
  .name = "quarantine",
  .mime_types = NULL,
  .data = NULL,
};
