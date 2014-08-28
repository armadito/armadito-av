#include <libumwsu/module.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

static const char *error_code_str(ERROR_CODE e)
{
  switch(e) {
#define M(E) case E: return #E
    M(E_NULL);
    M(E_SUCCESS);
    M(E_MALWARE);
    M(E_NOT_MALWARE);
    M(E_EAT_UNKNOWN);
    M(E_TFIDF_UNKNOWN);
    M(E_NOT_DECIDED);
    M(E_DOUBTFUL);
    M(E_ERROR);
    M(E_READING_ERROR);
    M(E_TEST_ERROR);
    M(E_DISTANCE_ERROR);
    M(E_FAILURE);
    M(E_CALLOC_ERROR);
    M(E_FILE_NOT_FOUND);
    M(E_FILE_EMPTY);
    M(E_NOT_MZ);
    M(E_NOT_PE);
    M(E_BAD_ARCHITECTURE);
    M(E_NO_ENTRY_POINT);
    M(E_EAT_EMPTY);
    M(E_IAT_EMPTY);
    M(E_SECTION_ERROR);
    M(E_DLL_NAME_ERROR);
    M(E_FUNCTION_NAME_ERROR);
    M(E_NAME_ERROR);
    M(E_CHECKSUM_ERROR);
    M(E_IAT_NOT_GOOD);
    M(E_EAT_NOT_GOOD);
    M(E_SECTION_EMPTY_NAME);
    M(E_NO_ENTRY);
    M(E_INVALID_ENTRY_POINT);
    M(E_NOT_ELF);
    M(E_SYMBOL_TABLE_EMPTY);
    M(E_BAD_FORMAT);
    M(E_PE_INIT);
    M(E_IAT);
    M(E_EAT);
  }

  return "UNKNOWN ERROR";
}

enum umwsu_status module5_2_scan(const char *path, void *mod_data, char **pmod_report)
{
  ERROR_CODE e;

  e = analyseElfFile((char *)path);

  *pmod_report = strdup(error_code_str(e));
  
  switch(e) {
  case E_MALWARE:
    return UMWSU_MALWARE;
  case E_NOT_MALWARE:
    return UMWSU_CLEAN;
  default:
    return UMWSU_IERROR;
  }

  return UMWSU_CLEAN;
}

enum umwsu_mod_status module5_2_init(void **pmod_data)
{
  if (initDB(MODULE_52_DBDIR "/database.elfdata", MODULE_52_DBDIR "/db_malicious.zip", MODULE_52_DBDIR "/db_safe.zip") == 0)
    return UMWSU_MOD_INIT_ERROR;

  fprintf(stderr, "Module 5.2 databases loaded from " MODULE_52_DBDIR "\n");

  return UMWSU_MOD_OK;
}

void module5_2_install(void)
{
  fprintf(stderr, "Module5_2 module installed\n");
}
