#include "h1-errors.h"

#include <stdio.h>

static ERROR_CODE currentError = ARMADITO_NULL;

VOID SetCurrentError(ERROR_CODE error) {
	currentError = error;
}

ERROR_CODE GetCurrentError(){
	return currentError;
}

const char *error_code_str(ERROR_CODE e)
{
  switch(e) {
#define M(E) case E: return #E
    M(ARMADITO_NULL);
    M(ARMADITO_SUCCESS);
    M(ARMADITO_MALWARE);
    M(ARMADITO_NOT_MALWARE);
    M(ARMADITO_EAT_UNKNOWN);
    M(ARMADITO_TFIDF_UNKNOWN);
    M(ARMADITO_NOT_DECIDED);
    M(ARMADITO_DOUBTFUL);
    M(ARMADITO_UNSUPPORTED_FILE);
    M(E_READING_ERROR);
    M(E_TEST_ERROR);
    M(E_DISTANCE_ERROR);
    M(E_FAILURE);
    M(E_CALLOC_ERROR);
    M(E_FILE_NOT_FOUND);
    M(E_FILE_EMPTY);
    M(E_FSTAT_ERROR);
    M(E_NOT_MZ);
    M(E_NOT_PE);
    M(E_BAD_ARCHITECTURE);
    M(ARMADITO_NO_SECTIONS);
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
    M(ARMADITO_INVALID_SECTION_NAME);
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
    M(ARMADITO_INVALID_SECTION);
    M(E_NOT_ELF);
    M(E_SYMBOL_TABLE_EMPTY);
    M(E_BAD_FORMAT);
    M(E_NO_KNOWN_SYMBOLS);
  }

  return "UNKNOWN ERROR";
}

CHAR* GetErrorCodeMsg(ERROR_CODE error){

	switch (error)
	{

	case ARMADITO_NULL:
		return "--ARMADITO_NULL--";

	case ARMADITO_SUCCESS:
		return "--ARMADITO_SUCCESS--";

	case ARMADITO_MALWARE:
		return "--ARMADITO_MALWARE--";

	case ARMADITO_NOT_MALWARE:
		return "--ARMADITO_NOT_MALWARE--";

	case ARMADITO_NOT_DECIDED:
		return "--ARMADITO_NOT_DECIDED--";

	case ARMADITO_EAT_UNKNOWN:
		return "--ARMADITO_EAT_UNKNOWN--";

	case ARMADITO_TFIDF_UNKNOWN:
		return "--ARMADITO_TFIDF_UNKNOWN--";

	case ARMADITO_UNSUPPORTED_FILE:
		return "--ARMADITO_UNSUPPORTED_FILE--";

	case ARMADITO_DOUBTFUL:
		return "--ARMADITO_DOUBTFUL--";

	case E_READING_ERROR:
		return "Error while reading the file";

	case E_TEST_ERROR:
		return "An error occured in a test function";

	case E_DISTANCE_ERROR:
		return "An error occured in a distance function";

	case E_CALLOC_ERROR:
		return "An error occured in a memory allocation";

	case E_FILE_NOT_FOUND:
		return "The file was not found";

	case E_FILE_EMPTY:
		return "The file is empty";

	case E_FSTAT_ERROR:
                return "Fstat error";

	case E_NOT_MZ:
		return "The file is not a MZ file";

	case E_NOT_PE:
		return "The file is not a PE file";

	case E_BAD_ARCHITECTURE:
		return "The file does not have an authorized architecture";

	case E_NO_ENTRY_POINT:
		return "The file does not have an entry point";

	case E_EAT_EMPTY:
		return "The file's EAT is empty";

	case E_IAT_EMPTY:
		return "The file's IAT is empty";

	case E_DLL_NAME_ERROR:
		return "A DLL name is not valid";

	case E_FUNCTION_NAME_ERROR:
		return "A function name is not valid";

	case E_CHECKSUM_ERROR:
		return "The file's checksum is not valid";

	case E_IAT_NOT_GOOD:
		return "The file's IAT is not valid";

	case E_EAT_NOT_GOOD:
		return "The file's EAT is not valid";

	case ARMADITO_INVALID_SECTION_NAME:
		return "The file has an invalid section name";

	case E_NO_ENTRY:
		return "A specified entry was not found";

	case E_INVALID_ENTRY_POINT:
		return "The entry point of the file is invalid";

	case E_INVALID_STUB:
		return "The stub of the file is invalid";

	case E_INVALID_TIMESTAMP:
		return "The timestamp of the file is invalid";

	case E_FIELDS_WITH_INVALID_VALUE:
		return "The header of the file is invalid";

	case E_INVALID_SIZE_OPT_HEADER:
		return "The size of the opt. header of the file is invalid";

	case E_EAT_INVALID_TIMESTAMP:
		return "The timestamp of the file's EAT is invalid";

	case E_IAT_INVALID_TIMESTAMP:
		return "The timestamp of the file's IAT is invalid";

	case E_EMPTY_VECTOR:
		return "A specified vector is empty";

	case E_INVALID_S_F_ALIGNMENT:
		return "The FileAlignment of the file is invalid";

	case ARMADITO_INVALID_SECTION:
		return "A specified section is invalid";

        case E_NOT_ELF:
 		return "This file is not a ELF";

	case E_SYMBOL_TABLE_EMPTY:
		return "Empty symbol table in this ELF file";

	case E_BAD_FORMAT:
		return "Bad ELF format";

	case E_NO_KNOWN_SYMBOLS:
		return "No known ELF Symbols";

	default:
		return "Unknown error";

	}
}
