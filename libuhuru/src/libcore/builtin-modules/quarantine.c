#include <libuhuru/core.h>
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

static enum uhuru_mod_status quarantine_init(struct uhuru_module *module)
{
  struct quarantine_data *qu_data = g_new(struct quarantine_data, 1);

  qu_data->quarantine_dir = NULL;
  qu_data->enable = 0;

  module->data = qu_data;

  return UHURU_MOD_OK;
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

void quarantine_callback(struct uhuru_report *report, void *callback_data)
{
  struct quarantine_data *qu_data = (struct quarantine_data *)callback_data;

  if (!qu_data->enable)
    return;

  switch(report->status) {
  case UHURU_UNDECIDED:
  case UHURU_CLEAN:
  case UHURU_UNKNOWN_FILE_TYPE:
  case UHURU_EINVAL:
  case UHURU_IERROR:
  case UHURU_SUSPICIOUS:
  case UHURU_WHITE_LISTED:
    return;
  }

  if (quarantine_do(qu_data, report->path) != -1)
    report->action |= UHURU_ACTION_QUARANTINE;
}

static enum uhuru_mod_status quarantine_conf_quarantine_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct quarantine_data *qu_data = (struct quarantine_data *)module->data;

  qu_data->quarantine_dir = strdup(argv[0]);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status quarantine_conf_enable(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct quarantine_data *qu_data = (struct quarantine_data *)module->data;

  qu_data->enable = !strcmp(argv[0], "yes") || !strcmp(argv[0], "1") ;

  return UHURU_MOD_OK;
}

static struct uhuru_conf_entry quarantine_conf_table[] = {
  { 
    .directive = "quarantine-dir", 
    .conf_fun = quarantine_conf_quarantine_dir, 
  },
  { 
    .directive = "enable", 
    .conf_fun = quarantine_conf_enable, 
  },
  { 
    .directive = NULL, 
    .conf_fun = NULL, 
  },
};

struct uhuru_module quarantine_module = {
  .init_fun = quarantine_init,
  .conf_table = quarantine_conf_table,
  .post_init_fun = NULL,
  .scan_fun = NULL,
  .close_fun = NULL,
  .name = "quarantine",
  .size = sizeof(struct quarantine_data),
};
