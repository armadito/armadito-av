#include "uh_errors.h"

#include <stdio.h>

static ERROR_CODE currentError = UH_NULL;

VOID SetCurrentError(ERROR_CODE error) {
	currentError = error;
}

ERROR_CODE GetCurrentError(){
	return currentError;
}

CHAR* GetErrorCodeMsg(ERROR_CODE error){

	switch (error)
	{

	case UH_NULL:
		return "--UH_NULL--";

	case UH_SUCCESS:
		return "--UH_SUCCESS--";

	case UH_MALWARE:
		return "--UH_MALWARE--";

	case UH_NOT_MALWARE:
		return "--UH_NOT_MALWARE--";

	case UH_NOT_DECIDED:
		return "--UH_NOT_DECIDED--";

	case UH_EAT_UNKNOWN:
		return "--UH_EAT_UNKNOWN--";

	case UH_TFIDF_UNKNOWN:
		return "--UH_TFIDF_UNKNOWN--";

	case UH_UNSUPPORTED_FILE:
		return "--UH_UNSUPPORTED_FILE--";

	case UH_DOUBTFUL:
		return "--UH_DOUBTFUL--";

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

	case UH_INVALID_SECTION_NAME:
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

	case UH_INVALID_SECTION:
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
