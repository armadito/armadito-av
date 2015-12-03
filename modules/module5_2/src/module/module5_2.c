#include <libuhuru/core.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "osdeps.h"
#include "uh_errors.h"
#include "UhuruStatic.h"

/*
  dirty module 5.2 code has no exported structure and uses only static data...
 */
static enum uhuru_mod_status module5_2_init(struct uhuru_module *module)
{
  return UHURU_MOD_OK;
}

static enum uhuru_mod_status module5_2_post_init(struct uhuru_module *module)
{
#ifndef WIN32
  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, "loading module 5.2 ELF databases from " MODULE5_2_DBDIR "/linux");

  if (initDB(MODULE5_2_DBDIR "/linux/database.elfdata", 
	     MODULE5_2_DBDIR "/linux/db_malicious.zip", 
	     MODULE5_2_DBDIR "/linux/db_safe.zip",
	     MODULE5_2_DBDIR "/linux/tfidf_m.dat",
	     MODULE5_2_DBDIR "/linux/tfidf_s.dat") != 0)
    return UHURU_MOD_INIT_ERROR;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, "Module 5.2 ELF databases loaded from " MODULE5_2_DBDIR "/linux");
#endif

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, "loading module 5.2 PE databases from " MODULE5_2_DBDIR "/windows");
  if (initDatabases(MODULE5_2_DBDIR "/windows/Database_malsain_2.zip",
		    MODULE5_2_DBDIR "/windows/Database_malsain_1.zip",
		    MODULE5_2_DBDIR "/windows/Database_sain_2.zip",
		    MODULE5_2_DBDIR "/windows/Database_sain_1.zip",
		    MODULE5_2_DBDIR "/windows/database_2.dat",
		    MODULE5_2_DBDIR "/windows/database_1.dat",
		    MODULE5_2_DBDIR "/windows/DBI_inf.dat",
		    MODULE5_2_DBDIR "/windows/DBI_sain.dat") != 0)
    return UHURU_MOD_INIT_ERROR;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, "module 5.2 PE databases loaded");

  return UHURU_MOD_OK;
}

// We receive a file descriptor
static enum uhuru_file_status module5_2_scan(struct uhuru_module *module, int fd, const char *path, const char *mime_type, char **pmod_report)
{
  ERROR_CODE e = UH_NULL;
  const char *virus_name = NULL;

  //printf("[i] Debug :: module 5_2_scan :: mime-type = %s\n",mime_type);

  // TODO change strcmp by strncmp
  // (FD) why????
  if (!strcmp(mime_type, "application/x-sharedlib")
      || !strcmp(mime_type, "application/x-object")
      || !strcmp(mime_type, "application/x-executable")) {	  
    e = analyseElfFile(fd, (char *)path);
    if (e == UH_MALWARE)
      virus_name = "Linux.Heuristic.Malware.Generic";
  } else if (!strcmp(mime_type, "application/x-dosexec") 
	     || !strcmp(mime_type, "application/x-msdownload")) {
    e = fileAnalysis(fd, (char *)path);
    if (e == UH_MALWARE)
      virus_name = "Win.Heuristic.Malware.Generic";
  }

  switch(e) {
  case UH_MALWARE:
    /* even if virus_name is a statically allocated string, it must be returned in a dynamically allocated string */
    /* because it will be free()d by the calling code */
    *pmod_report = os_strdup(virus_name);
    return UHURU_MALWARE;
  case UH_NOT_MALWARE:
    return UHURU_CLEAN;
  case UH_NOT_DECIDED:
  case UH_DOUBTFUL:
    return UHURU_UNDECIDED;
  }

  return UHURU_IERROR;
}

static enum uhuru_mod_status module5_2_close(struct uhuru_module *module)
{
  return UHURU_MOD_OK;
}

/* FIXME: one day, add bases status */
static enum uhuru_update_status module5_2_info(struct uhuru_module *module, struct uhuru_module_info *info)
{
  info->base_infos = (struct uhuru_base_info **)malloc(sizeof(struct uhuru_base_info *));
  info->base_infos[0] = NULL;

  info->update_date = os_strdup("2014-09-01T09:30:00Z");

  return UHURU_UPDATE_OK;
}

struct uhuru_module module = {
  .init_fun = module5_2_init,
  .conf_table= NULL,
  .post_init_fun = module5_2_post_init,
  .scan_fun = module5_2_scan,
  .close_fun = module5_2_close,
  .info_fun = module5_2_info,
  .name = "module5_2",
};
