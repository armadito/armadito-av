#include <libumwsu/module.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uh_errors.h"
#include "UhuruStatic.h"

const char *module5_2_mime_types[] = {
  "application/x-sharedlib",
  "application/x-object",
  "application/x-executable",
  "application/x-msdos-program",
  "application/x-dosexec",
  NULL,
};

static const char *error_code_str(ERROR_CODE e)
{
  switch(e) {
#define M(E) case E: return #E
    M(UH_NULL);
    M(UH_SUCCESS);
    M(UH_MALWARE);
    M(UH_NOT_MALWARE);
    M(UH_EAT_UNKNOWN);
    M(UH_TFIDF_UNKNOWN);
    M(UH_NOT_DECIDED);
    M(UH_DOUBTFUL);
    M(UH_UNSUPPORTED_FILE);
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
    M(UH_NO_SECTIONS);
    M(E_NO_ENTRY_POINT);
    M(E_EAT_EMPTY);
    M(E_IAT_EMPTY);
    M(E_SECTION_ERROR);
    M(E_DLL_NAME_ERROR);
    M(E_NAME_ERROR);
    M(E_CHECKSUM_ERROR);
    M(E_IAT_NOT_GOOD);
    M(E_EAT_NOT_GOOD);
    M(E_FUNCTION_NAME_ERROR);
    M(E_NO_ENTRY);
    M(UH_INVALID_SECTION_NAME);
    M(E_INVALID_ENTRY_POINT);
    M(E_INVALID_STUB);
    M(E_INVALID_TIMESTAMP);
    M(E_FIELDS_WITH_INVALID_VALUE);
    M(E_INVALID_SIZE_OPT_HEADER);
    M(E_EAT_INVALID_TIMESTAMP);
    M(E_IAT_INVALID_TIMESTAMP);
    M(E_EMPTY_VECTOR);
    M(E_INVALID_STRUCTURE);
    M(E_INVALID_NUMBER_RVA_SIZES);
    M(E_INVALID_FILE_SIZE);
    M(E_HEADER_NOT_GOOD);
    M(E_INVALID_S_F_ALIGNMENT);
    M(UH_INVALID_SECTION);
    M(E_NOT_ELF);
    M(E_SYMBOL_TABLE_EMPTY);
    M(E_BAD_FORMAT);
    M(E_NO_KNOWN_SYMBOLS);
  }

  return "UNKNOWN ERROR";
}

enum umwsu_status module5_2_scan(const char *path, const char *mime_type, void *mod_data, char **pmod_report)
{
  ERROR_CODE e;

  if (!strcmp(mime_type, "application/x-sharedlib")
      || !strcmp(mime_type, "application/x-object")
      || !strcmp(mime_type, "application/x-executable")) {
    e = analyseElfFile((char *)path);
  } else if (!strcmp(mime_type, "application/x-dosexec") ) {
    e = fileAnalysis((char *)path);
  }

  *pmod_report = strdup(error_code_str(e));
  
#if 0
  fprintf(stderr, "module 5.2: %s %s\n", path, error_code_str(e));
#endif

  switch(e) {
  case UH_MALWARE:
    return UMWSU_MALWARE;
  case UH_NOT_MALWARE:
    return UMWSU_CLEAN;
  case UH_NOT_DECIDED:
  case UH_DOUBTFUL:
    return UMWSU_UNDECIDED;
  }

  return UMWSU_IERROR;
}

enum umwsu_mod_status module5_2_init(void **pmod_data)
{
  if (initDB(MODULE5_2_DBDIR "/Linux/database.elfdata", 
	     MODULE5_2_DBDIR "/Linux/db_malicious.zip", 
	     MODULE5_2_DBDIR "/Linux/db_safe.zip",
	     MODULE5_2_DBDIR "/Linux/tfidf_m.dat",
	     MODULE5_2_DBDIR "/Linux/tfidf_s.dat") != 0)
    return UMWSU_MOD_INIT_ERROR;

  fprintf(stderr, "Module 5.2 ELF databases loaded from " MODULE5_2_DBDIR "/elf\n");

  if (initDatabases(MODULE5_2_DBDIR "/Windows/Database_malsain_2.zip",
		    MODULE5_2_DBDIR "/Windows/wip/Database_malsain_1.zip",
		    MODULE5_2_DBDIR "/Windows/Database_sain_2.zip",
		    MODULE5_2_DBDIR "/Windows/wip/Database_sain_1.zip",
		    MODULE5_2_DBDIR "/Windows/database_2.dat",
		    MODULE5_2_DBDIR "/Windows/wip/database_1.dat",
		    MODULE5_2_DBDIR "/Windows/DBI_inf.dat",
		    MODULE5_2_DBDIR "/Windows/DBI_sain.dat") != 0)
    return UMWSU_MOD_INIT_ERROR;

  fprintf(stderr, "Module 5.2 Windows PE databases loaded from " MODULE5_2_DBDIR "/pe\n");

  return UMWSU_MOD_OK;
}

void module5_2_install(void)
{
  fprintf(stderr, "Module 5.2 installed\n");
}
