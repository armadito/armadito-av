#include <libumwsu/module.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "errors.h"

#define LEARN_TO_PROTOTYPE_EXPORTED_FUNCTIONS
#ifdef LEARN_TO_PROTOTYPE_EXPORTED_FUNCTIONS
int initDB(char* dbName, char* malicousDbName, char* notMalicousDbName);
void freeDB(void);
ERROR_CODE analyseElfFile(char* fileName);
#endif

const char *module5_2_mime_types[] = {
  "application/x-sharedlib",
  "application/x-object",
  "application/x-executable",
  NULL,
};

enum umw_status module5_2_scan(const char *path, void *mod_data)
{
  switch(analyseElfFile((char *)path)) {
  case E_MALWARE:
    return UMW_MALWARE;
  case E_NOT_MALWARE:
    return UMW_CLEAN;
  default:
    return UMW_IERROR;
  }

  return UMW_CLEAN;
}

enum umw_mod_status module5_2_init(void **pmod_data)
{
  if (initDB(MODULE_52_DBDIR "/database.elfdata", MODULE_52_DBDIR "/db_malicious.zip", MODULE_52_DBDIR "/db_safe.zip") == 0)
    return UMW_MOD_INIT_ERROR;

  fprintf(stderr, "Module 5.2 databases loaded from " MODULE_52_DBDIR "\n");

  return UMW_MOD_OK;
}

void module5_2_install(void)
{
  fprintf(stderr, "Module5_2 module installed\n");
}
