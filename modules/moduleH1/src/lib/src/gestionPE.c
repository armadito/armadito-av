/**
 * @file   gestionPE.c
 *
 * implements the reading of files with MZ-PE format
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> // for toupper
#include <string.h>
#include <time.h> // for the computation of the timestamp

#include "gestionPE.h"
#include "utils.h"
#include "osdeps.h"

/**
 * convert a CHAR* to upper case
 * @param  sPtr the CHAR* to convert
 */
VOID ConvertToUpperCase(CHAR* sPtr){
	while (*sPtr != '\0'){
		*sPtr = (CHAR)toupper((UCHAR)*sPtr);
		sPtr++;
	}
}

/**
 * check if the passed timestamp is valid, ie. not superior to the actual timestamp
 * @param  timeStamp the timestamp to check
 * @return           TRUE if it is valid, FALSE if it is not
 */
BOOLEAN IsValidTimeStamp(DWORD timeStamp){
	time_t timer;
	timer = time(NULL);

	if (timeStamp == 0)
	{
		return TRUE;
	}

	if ((DWORD)timer < timeStamp){
		return FALSE;
	}

	return TRUE;
}

// Purpose:
//		This function sort an array of QWORD using the bubble sort algorithm.
//
// Parameters:
//		_in_ t : the array to be sorted
//		_in_ n : the size of the array
//
// Return:
//		void
VOID BubbleSort(QWORD *t, QWORD n){
	QWORD j = 0;
	QWORD tmp = 0;
	BOOLEAN Unsorted = TRUE;

	if (t == NULL){
		return;
	}

	while (Unsorted == TRUE){
		Unsorted = FALSE;
		for (j = 0; j < n - 1; j++){
			if (t[j] > t[j + 1]){
				tmp = t[j + 1];
				t[j + 1] = t[j];
				t[j] = tmp;
				Unsorted = TRUE;
			}
		}
	}
}

// Purpose:
//		This function test a function name and check if it contains only valid characters.
//
// Parameters:
//		_in_ szFileName : the name to check
//		_in_ size : the size of the name
//
// Return:
//		TRUE : if the name is valid.
//		FALSE : if the name is not valid.
BOOLEAN IsAValidFunctionName(UCHAR* szFileName, QWORD size){
	DWORD i;

	if (size == 0){
		return FALSE;
	}

	/* for each character, test if it is a authorized character */
	for (i = 0; i < size || szFileName[i] != 0; i++){
		if (!((szFileName[i] >= 48 && szFileName[i] <= 57) || /* Is a number */
			(szFileName[i] >= 63 && szFileName[i] <= 90) || /* Is an uppercase character + ? and @ */
			(szFileName[i] == 60) || /* Is < */
			(szFileName[i] == 62) || /* Is > */
			(szFileName[i] == 95) || /* Is underscore */
			(szFileName[i] >= 97 && szFileName[i] <= 122) || /* Is a lowercase character */
			(szFileName[i] == 36))){						   /* Is a dollar */
			return FALSE;
		}
	}

	return TRUE;
}

// Purpose:
//		This function test a DLL name and check if it contains only valid characters.
//
// Parameters:
//		_in_ szFileName : the name to check
//		_in_ size : the size of the name
//
// Return:
//		TRUE : if the name is valid.
//		FALSE : if the name is not valid.
BOOLEAN IsAValidDllName(UCHAR* szFileName, QWORD size){
	DWORD i;

	if (size == 0){
		return FALSE;
	}

	/* for each character, test if it is a authorized character */
	for (i = 0; i < size || szFileName[i] != 0; i++){
		if (!((szFileName[i] >= 48 && szFileName[i] <= 57) ||  /* Is a number */
			(szFileName[i] >= 63 && szFileName[i] <= 90) ||  /* Is an uppercase character + ? and @ */
			(szFileName[i] == 95) ||  /* Is underscore */
			(szFileName[i] == 40) ||  /* Is ( */
			(szFileName[i] == 41) ||  /* Is ) */
			(szFileName[i] == 43) ||  /* Is + */
			(szFileName[i] >= 97 && szFileName[i] <= 122) ||  /* Is a lowercase character */
			(szFileName[i] >= 45 && szFileName[i] <= 46) ||  /* Is a dot or a minus */
			(szFileName[i] == 36))){							/* Is a dollar */
			return FALSE;
		}
	}

	return TRUE;
}

/**
* check if the address passed in argument is a valid address
* @param  Pe      the PORTABLE_EXECUTABLE representing the file
* @param  address the address to test
* @return         TRUE or FALSE
*/
BOOLEAN PeIsValidAddress(PPORTABLE_EXECUTABLE Pe, ULONG_PTR address){
	return !(address > (Pe->FileSize + Pe->BaseAddress) || address < Pe->BaseAddress);
}

ERROR_CODE PeInit(PPORTABLE_EXECUTABLE Pe, int fd, CHAR* filename){
	DWORD MagicWord = 0;
	ULONG_PTR Offset = 0;
	int stat_errno;

	Pe->ImagesSectionHeader = NULL;

	/* computation of the file size */
	Pe->FileSize = (DWORD)os_file_size(fd, &stat_errno);
	if (Pe->FileSize == 0){
		return E_FILE_EMPTY;
	}
	else if(Pe->FileSize == -1){
		return E_FSTAT_ERROR;
	}	

	if (Pe->FileSize < sizeof(PIMAGE_DOS_HEADER)){
		return E_INVALID_FILE_SIZE;
	}

	/* creation of the ULONG_PTR used to store the file */
	Pe->BaseAddress = (ULONG_PTR)calloc(Pe->FileSize + 1, sizeof(UCHAR));
	if (Pe->BaseAddress == NULL){
		return E_CALLOC_ERROR;
	}

	/* file reading into Pe->BaseAddress */
	if (os_read(fd, (PVOID)Pe->BaseAddress, Pe->FileSize) == -1){
		return E_READING_ERROR;
	}

	/* reading of the DOS header and test if e_magic is set to the MZ signature */
	Pe->ImageDosHeader = *(PIMAGE_DOS_HEADER)((PVOID)(Pe->BaseAddress));
	if (Pe->ImageDosHeader.e_magic != IMAGE_DOS_SIGNATURE){
		return E_NOT_MZ;
	}

	/* reading of the magic word and test if it is set to the PE signature */
	if (!PeIsValidAddress(Pe, Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew)){
		return E_HEADER_NOT_GOOD;
	}

	MagicWord = *(PDWORD)((PVOID)(Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew));
	if (MagicWord != IMAGE_NT_SIGNATURE) {
		return E_NOT_PE;
	}

	/* reading the of PE header */
	if (!PeIsValidAddress(Pe, Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew + sizeof(DWORD))){
		return E_HEADER_NOT_GOOD;
	}
	Pe->ImageFileHeader = *(PIMAGE_FILE_HEADER)((PVOID)(Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew + sizeof(DWORD)));

	/* reading of the machine value  : only x86 and x64 are authorized */
	Pe->Machine = Pe->ImageFileHeader.Machine;
	if (!(Pe->Machine == IMAGE_FILE_MACHINE_I386 || Pe->Machine == IMAGE_FILE_MACHINE_AMD64)){
		return E_BAD_ARCHITECTURE;
	}

	/* reading of the OptionalHeader and computation of the offset of the section header */
	if (Pe->ImageFileHeader.SizeOfOptionalHeader != 0) {
		if (Pe->ImageFileHeader.Machine == IMAGE_FILE_MACHINE_I386){
			Pe->ImageOptionalHeader32 = *(PIMAGE_OPTIONAL_HEADER32)((PVOID)(Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER)));
			Offset = Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew + (ULONG_PTR)FIELD_OFFSET32(IMAGE_NT_HEADERS32, OptionalHeader) + Pe->ImageFileHeader.SizeOfOptionalHeader;
		}
		else if (Pe->ImageFileHeader.Machine == IMAGE_FILE_MACHINE_AMD64){
			Pe->ImageOptionalHeader64 = *(PIMAGE_OPTIONAL_HEADER64)((PVOID)(Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER)));
			Offset = Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew + (ULONG_PTR)FIELD_OFFSET64(IMAGE_NT_HEADERS64, OptionalHeader) + Pe->ImageFileHeader.SizeOfOptionalHeader;
		}
	}

	if (!PeIsValidAddress(Pe, Offset)){
		return E_HEADER_NOT_GOOD;
	}
	/* reading of the ImagesSectionHeader */
	Pe->ImagesSectionHeader = (PIMAGE_SECTION_HEADER)((PVOID)(Offset));

	return ARMADITO_SUCCESS;
}

/**
* check if the file has sections, and if one of the sections of the file has an empty name
* @param  Pe the PORTABLE_EXECUTABLE representing the file
* @return    an ERROR_CODE value between : ARMADITO_INVALID_SECTION_NAME, ARMADITO_NO_SECTIONS and ARMADITO_SUCCESS
*/
ERROR_CODE PeHasEmptySectionName(PPORTABLE_EXECUTABLE Pe){
	DWORD i = 0;
	BYTE j = 0;

	/* first, check if there is sections in the file */
	if (Pe->ImagesSectionHeader == NULL || Pe->ImageFileHeader.NumberOfSections == 0){
		return ARMADITO_NO_SECTIONS;
	}

	/* check if the section header contains a section with a name of size 0 or 1 */
	for (i = 0; i < Pe->ImageFileHeader.NumberOfSections; i++){
		if ((CHAR*)Pe->ImagesSectionHeader[i].Name == NULL){
			return ARMADITO_INVALID_SECTION_NAME;
		}

		if (strlen((CHAR*)Pe->ImagesSectionHeader[i].Name) <= 1){
			return ARMADITO_INVALID_SECTION_NAME;
		}

		/* check if the first element of the name is a-z, A-Z, 0-9, '.', '/' or '_' */
		if (!((Pe->ImagesSectionHeader[i].Name[0] >= 48 && Pe->ImagesSectionHeader[i].Name[0] <= 57) ||
			(Pe->ImagesSectionHeader[i].Name[0] >= 65 && Pe->ImagesSectionHeader[i].Name[0] <= 90) ||
			(Pe->ImagesSectionHeader[i].Name[0] == 95) || /* Is underscore */
			(Pe->ImagesSectionHeader[i].Name[0] == 46) || /* Is point */
			(Pe->ImagesSectionHeader[i].Name[0] == 47) || /* Is / */
			(Pe->ImagesSectionHeader[i].Name[0] >= 97 && Pe->ImagesSectionHeader[i].Name[0] <= 122))){
			return ARMADITO_INVALID_SECTION_NAME;
		}

		/*
		* check if the rest of the name is composed only of a-z, A-Z, 0-9, ':' and '_'
		* since the documentation say that if the name is 8 char, it is not null terminated, the maximum number of
		* characters to be tested is 7
		*/
		for (j = 1; j < MIN(strlen((CHAR*)Pe->ImagesSectionHeader[i].Name), 7); j++)
		{
			if (!((Pe->ImagesSectionHeader[i].Name[j] >= 48 && Pe->ImagesSectionHeader[i].Name[j] <= 57) ||
				(Pe->ImagesSectionHeader[i].Name[j] >= 65 && Pe->ImagesSectionHeader[i].Name[j] <= 90) ||
				(Pe->ImagesSectionHeader[i].Name[j] == 95) || /* Is underscore */
				(Pe->ImagesSectionHeader[i].Name[0] == 46) || /* Is point */
				(Pe->ImagesSectionHeader[i].Name[0] == 45) || /* Is hyphen */
				(Pe->ImagesSectionHeader[i].Name[j] == 58) || /* Is semi column */
				(Pe->ImagesSectionHeader[i].Name[j] >= 97 && Pe->ImagesSectionHeader[i].Name[j] <= 122))){
				return ARMADITO_INVALID_SECTION_NAME;
			}
		}
	}

	return ARMADITO_SUCCESS;
}

/**
 * implements the checksum algorithm in order to compute the checksum of a file
 * @param  CheckSum the base checksum
 * @param  fileBase the address of the file
 * @param  length   the length of the file
 * @return          the checksum of the file
 */
DWORD ComputeChecksum(DWORD CheckSum, VOID *fileBase, DWORD length) {
	DWORD *Data;
	DWORD sum;
	if (length && fileBase != NULL) {
		Data = (DWORD *)fileBase;
		do {
			sum = *(WORD *)Data + CheckSum;
			Data = (DWORD *)((CHAR *)Data + 2);
			CheckSum = (WORD)sum + (sum >> 16);
		} while (--length);
	}
	return CheckSum + (CheckSum >> 16);
}

/**
* compute the checksum of a PE file in order to compare it to the written checksum in the file
* @param  Pe the PORTABLE_EXECUTABLE representing the file
* @return    an ERROR_CODE value between : E_CHECKSUM_ERROR and ARMADITO_SUCCESS
*/
ERROR_CODE PeHasValidChecksum(PPORTABLE_EXECUTABLE Pe) {
	VOID *RemainData;
	DWORD RemainDataSize = 0, PeHeaderSize = 0, PeHeaderCheckSum = 0, FileCheckSum = 0, writtenChksm = 0, realChksm = 0;
	PIMAGE_NT_HEADERS32 NtHeaders = NULL;
	PIMAGE_NT_HEADERS64 NtHeaders64 = NULL;

	if (Pe->ImageFileHeader.SizeOfOptionalHeader != 0) {
		if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
			writtenChksm = Pe->ImageOptionalHeader32.CheckSum; /* retrieve checksum value in optional header */
		}
		else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
			writtenChksm = Pe->ImageOptionalHeader64.CheckSum; /* retrieve checksum value in optional header */
		}
	}

	/* if the written checksum is 0, no tests are done (because some safe file can have a null checksum) */
	if (writtenChksm == 0){
		return ARMADITO_SUCCESS;
	}

	/**
	 * In order to compute the checksum of a file, the file is cut in two parts : the part before the checksum field in  the NTHeader,
	 * and the part after the checksum field. For each part, the checksum is computed using the ComputeChecksum function.
	 * http://litao.me/post/2011-06-28-PE-File-Checksum-Algorithm.html
	 * can be computed with MapFileAndCheckSum if using the WindowsAPI.
	 */
	if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
		NtHeaders = (PIMAGE_NT_HEADERS32)((PVOID)(Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew));

		if (NtHeaders != NULL) {
			PeHeaderSize = (DWORD)((ULONG_PTR)&NtHeaders->OptionalHeader.CheckSum - (ULONG_PTR)Pe->BaseAddress);
			RemainDataSize = (Pe->FileSize - PeHeaderSize - 4/*checksum field*/) >> 1;
			RemainData = &NtHeaders->OptionalHeader.Subsystem; /*field after checksum*/
			PeHeaderCheckSum = ComputeChecksum(0, (PVOID)Pe->BaseAddress, PeHeaderSize >> 1);
			FileCheckSum = ComputeChecksum(PeHeaderCheckSum, RemainData, RemainDataSize);

			if (Pe->FileSize & 1){
				FileCheckSum += (CHAR)*((CHAR *)Pe->BaseAddress + Pe->FileSize - 1);
			}
		}
		else{
			FileCheckSum = 0;
		}
	}
	else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
		NtHeaders64 = (PIMAGE_NT_HEADERS64)((PVOID)(Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew));

		if (NtHeaders64 != NULL) {
			PeHeaderSize = (DWORD)((ULONG_PTR)&NtHeaders64->OptionalHeader.CheckSum - (ULONG_PTR)Pe->BaseAddress);
			RemainDataSize = (Pe->FileSize - PeHeaderSize - 4/*checksum field*/) >> 1;
			RemainData = &NtHeaders64->OptionalHeader.Subsystem; /*field after checksum*/
			PeHeaderCheckSum = ComputeChecksum(0, (PVOID)Pe->BaseAddress, PeHeaderSize >> 1);
			FileCheckSum = ComputeChecksum(PeHeaderCheckSum, RemainData, RemainDataSize);

			if (Pe->FileSize & 1){
				FileCheckSum += (CHAR)*((CHAR *)Pe->BaseAddress + Pe->FileSize - 1);
			}
		}
		else {
			FileCheckSum = 0;
		}
	}

	/* in order to have the real checksum, the size of the file is added to the file checksum */
	realChksm = FileCheckSum + Pe->FileSize;

	/**
	 * test if the computed checksum is equal to the written checksum in the NTHeader
	 * for some reasons, sometimes 0xff00 must be added in order to have the same value,
	 * even for safe files
	 */
	if (/*(writtenChksm + 0xff00) != realChksm && */writtenChksm != realChksm && writtenChksm != 0){
		return E_CHECKSUM_ERROR;
	}

	return ARMADITO_SUCCESS;
}

/**
* check if the file has the valid dos stub in its content
* @param  Pe the PORTABLE_EXECUTABLE representing the file
* @return    an ERROR_CODE value between : E_INVALID_STUB if the stub is invalid or not present
*               							ARMADITO_SUCCESS if the stub is correct
*/
ERROR_CODE PeHasValidDOSStub(PPORTABLE_EXECUTABLE Pe){
	/* test if the file has a STUB (which is between the dos header and the file header)
	   so no STUB => e_lfanew value is the next byte */
	if (Pe->ImageDosHeader.e_lfanew == 0x40)
	{
		return ARMADITO_SUCCESS;
	}

	/*TODO : Faire des recherches sur le stub alternatif !! */
	CHAR fileStub[DOS_STUB_SIZE];
	CHAR fileStubAlt[ALT_DOS_STUB_SIZE];

	os_strncpy(fileStub,DOS_STUB_SIZE, (CHAR*)(Pe->BaseAddress + DOS_STUB_OFFSET), DOS_STUB_SIZE - 1);
	os_strncpy(fileStubAlt,ALT_DOS_STUB_SIZE, (CHAR*)(Pe->BaseAddress + ALT_DOS_STUB_OFFSET), ALT_DOS_STUB_SIZE - 1);
	fileStub[DOS_STUB_SIZE - 1] = 0;
	fileStubAlt[ALT_DOS_STUB_SIZE - 1] = 0;
	if (strcmp(DOS_STUB, fileStub) != 0 && strcmp(ALT_DOS_STUB, fileStubAlt) != 0){
		return E_INVALID_STUB;
	}

	return ARMADITO_SUCCESS;
}

/**
* check if the timestamp in the file header is valid
* @param  Pe the PORTABLE_EXECUTABLE representing the file
* @return    an ERROR_CODE value between : E_INVALID_TIMESTAMP if the timestamp is invalid
*               							ARMADITO_SUCCESS if the timestamp is correct
*/
ERROR_CODE PeFileHeaderHasValidTimestamp(PPORTABLE_EXECUTABLE Pe){
	if (!IsValidTimeStamp(Pe->ImageFileHeader.TimeDateStamp)){
		return E_INVALID_TIMESTAMP;
	}

	return ARMADITO_SUCCESS;
}

/**
* check if the NumberOfRvaAndSizes field of the opt header has the right value
* @param  Pe the PORTABLE_EXECUTABLE representing the file
* @return    an ERROR_CODE value between : E_INVALID_NUMBER_RVA_SIZES if the value is incorrect
*               							ARMADITO_SUCCESS if the value is correct
*/
ERROR_CODE PeGoodNumberOfRvaAndSizes(PPORTABLE_EXECUTABLE Pe){
	if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
		if (Pe->ImageOptionalHeader32.NumberOfRvaAndSizes != IMAGE_NUMBEROF_DIRECTORY_ENTRIES){
			return E_INVALID_NUMBER_RVA_SIZES;
		}
	}
	else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
		if (Pe->ImageOptionalHeader64.NumberOfRvaAndSizes != IMAGE_NUMBEROF_DIRECTORY_ENTRIES){
			return E_INVALID_NUMBER_RVA_SIZES;
		}
	}

	return ARMADITO_SUCCESS;
}

/**
* some fields in the headers must be 0, so this function checks the value of those fields
* @param  Pe the PORTABLE_EXECUTABLE representing the file
* @return    an ERROR_CODE value between : E_FIELDS_WITH_INVALID_VALUE if a field has an invalid value
*               							ARMADITO_SUCCESS if all is correct
*/
ERROR_CODE PeHasWantedFieldsToNull(PPORTABLE_EXECUTABLE Pe){
	// It appears the GCC does not set those values to 0 (maybe because they are just supposed to be null)
	// So these two tests can't be used if we don't want to class all the elf compiled with GCC as malware
#if 0
	if (Pe->ImageFileHeader.PointerToSymbolTable != 0/* && Pe->ImageFileHeader.NumberOfSymbols != 0*/){
		return E_FIELDS_WITH_INVALID_VALUE;
	}
	// TODO TOFIX only one safe file (hha.dll) does not match this condition, why is a good question, since its required in any PE file
	// cf. tests_systeme_windows.txt
	if (Pe->ImageFileHeader.NumberOfSymbols != 0){
		return E_FIELDS_WITH_INVALID_VALUE;
	}
#endif

	if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
		if (Pe->ImageOptionalHeader32.Win32VersionValue != 0){
			return E_FIELDS_WITH_INVALID_VALUE;
		}
		else if (Pe->ImageOptionalHeader32.LoaderFlags != 0){
			return E_FIELDS_WITH_INVALID_VALUE;
		}
	}
	else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
		if (Pe->ImageOptionalHeader64.Win32VersionValue != 0){
			return E_FIELDS_WITH_INVALID_VALUE;
		}
		else if (Pe->ImageOptionalHeader64.LoaderFlags != 0){
			return E_FIELDS_WITH_INVALID_VALUE;
		}
	}

	return ARMADITO_SUCCESS;
}

/**
* check if the SizeOfOptionalHeader field of the file header is equal to the size of the opt. header struct
* @param  Pe the PORTABLE_EXECUTABLE representing the file
* @return    an ERROR_CODE value between : E_INVALID_SIZE_OPT_HEADER if the value is incorrect
*               							ARMADITO_SUCCESS if the value is correct
*/
ERROR_CODE PeFileHeaderHasGoodSizeOfOptionalHeader(PPORTABLE_EXECUTABLE Pe){
	if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
		if (Pe->ImageFileHeader.SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER32)){
			return E_INVALID_SIZE_OPT_HEADER;
		}
	}
	else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
		if (Pe->ImageFileHeader.SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER64)){
			return E_INVALID_SIZE_OPT_HEADER;
		}
	}

	return ARMADITO_SUCCESS;
}

/**
* check if the FileAlignment field of the file header is valid depending on the documentation :
* SectionAlignment :
*		The alignment (in bytes) of sections when they are loaded into memory. It must be greater than or equal to
*		FileAlignment. The default is the page size for the architecture
* FileAlignment :
*		The alignment factor (in bytes) that is used to align the raw data of sections in the image file.
*		The value should be a power of 2 between 512 and 64 K, inclusive. The default is 512. If the SectionAlignment
*		is less than the architectures page size, then FileAlignment must match SectionAlignment.
* @param  Pe the PORTABLE_EXECUTABLE representing the file
* @return    an ERROR_CODE value between : E_INVALID_S_F_ALIGNMENT if the value is incorrect
*               							ARMADITO_SUCCESS if the value is correct
*/
ERROR_CODE PeHasValidSFAlignment(PPORTABLE_EXECUTABLE Pe){
	if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
		if (Pe->ImageOptionalHeader32.SectionAlignment < USN_PAGE_SIZE)
		{
			if (Pe->ImageOptionalHeader32.FileAlignment != Pe->ImageOptionalHeader32.SectionAlignment)
			{
				return E_INVALID_S_F_ALIGNMENT;
			}
		}
		else
		{
			if (Pe->ImageOptionalHeader32.FileAlignment > Pe->ImageOptionalHeader32.SectionAlignment)
			{
				return E_INVALID_S_F_ALIGNMENT;
			}
			else if (Pe->ImageOptionalHeader32.FileAlignment < 0x200 ||
				Pe->ImageOptionalHeader32.FileAlignment > 0x10000 ||
				(Pe->ImageOptionalHeader32.FileAlignment & 1) == 1)
			{
				return E_INVALID_S_F_ALIGNMENT;
			}
		}
	}
	else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
		if (Pe->ImageOptionalHeader64.SectionAlignment < USN_PAGE_SIZE)
		{
			if (Pe->ImageOptionalHeader64.FileAlignment != Pe->ImageOptionalHeader64.SectionAlignment)
			{
				return E_INVALID_S_F_ALIGNMENT;
			}
		}
		else
		{
			if (Pe->ImageOptionalHeader64.FileAlignment > Pe->ImageOptionalHeader64.SectionAlignment)
			{
				return E_INVALID_S_F_ALIGNMENT;
			}
			else if (Pe->ImageOptionalHeader64.FileAlignment < 0x200 ||
				Pe->ImageOptionalHeader64.FileAlignment > 0x10000 ||
				(Pe->ImageOptionalHeader64.FileAlignment & 1) == 1)
			{
				return E_INVALID_S_F_ALIGNMENT;
			}
		}
	}

	return ARMADITO_SUCCESS;
}

/**
* Check if the sections of the file are good according to the documentation on
* - the PointerToRawData field
* - the SizeOfRawData field
* - the Misc.VirtualSize field
* @param  Pe the PORTABLE_EXECUTABLE representing the file
* @return    an ERROR_CODE value between : ARMADITO_INVALID_SECTION if a value is incorrect
*               							ARMADITO_SUCCESS if the sections are good
*/
ERROR_CODE PeHasValidSectionsData(PPORTABLE_EXECUTABLE Pe){
	DWORD i = 0;

	if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
		while (i < Pe->ImageFileHeader.NumberOfSections){
			if (Pe->ImagesSectionHeader[i].PointerToRawData % Pe->ImageOptionalHeader32.FileAlignment != 0 ||
				Pe->ImagesSectionHeader[i].SizeOfRawData % Pe->ImageOptionalHeader32.FileAlignment != 0 ||
				Pe->ImagesSectionHeader[i].Misc.VirtualSize == 0)
			{
				return ARMADITO_INVALID_SECTION;
			}

			i++;
		}
	}
	else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
		while (i < Pe->ImageFileHeader.NumberOfSections){
			if (Pe->ImagesSectionHeader[i].PointerToRawData % Pe->ImageOptionalHeader64.FileAlignment != 0 ||
				Pe->ImagesSectionHeader[i].SizeOfRawData % Pe->ImageOptionalHeader64.FileAlignment != 0 ||
				Pe->ImagesSectionHeader[i].Misc.VirtualSize == 0)
			{
				return ARMADITO_INVALID_SECTION;
			}

			i++;
		}
	}

	return ARMADITO_SUCCESS;
}

/**
* check if the file has the required format and structure according to the PE documentation
* @param  Pe the PORTABLE_EXECUTABLE representing the file
* @return    an ERROR_CODE value between : E_INVALID_STRUCTURE and ARMADITO_SUCCESS
*/
ERROR_CODE PeHasValidStructure(PPORTABLE_EXECUTABLE Pe){
	/* check if the sections of the file are good */
	if (PeHasEmptySectionName(Pe) == ARMADITO_INVALID_SECTION_NAME ||
		PeHasEmptySectionName(Pe) == ARMADITO_NO_SECTIONS ||
		PeHasValidSectionsData(Pe) == ARMADITO_INVALID_SECTION){
		SetCurrentError(ARMADITO_INVALID_SECTION);
		return E_INVALID_STRUCTURE;
	}

	// Some file files all the files in the mozilla API does not have a correct checksum.
	// So this test loses all its value thanks to them (and because it is said in the doc
	// that the checksum does not need to be correct for every type of file)
#if 0
	/* check if the checksum of the file is valid */
	else if (PeHasValidChecksum(Pe) == E_CHECKSUM_ERROR){
		SetCurrentError(E_CHECKSUM_ERROR);
		return E_INVALID_STRUCTURE;
	}
#endif

	// it appears that some exe and dll of NVidia and VirtualBox have a custom STUB message, so they are
	// considered malicious when in fact they are not.
#if 0
	/* check if the DOS stub of the file is correct */
	else if (PeHasValidDOSStub(Pe) == E_INVALID_STUB){
		SetCurrentError(E_INVALID_STUB);
		return E_INVALID_STRUCTURE;
	}
#endif

	/* check if the timestamp of the file is valid */
	else if (PeFileHeaderHasValidTimestamp(Pe) == E_INVALID_TIMESTAMP){
		SetCurrentError(E_INVALID_TIMESTAMP);
		return E_INVALID_STRUCTURE;
	}
	/* check if specifics fields that must be 0 are 0 */
	else if (PeHasWantedFieldsToNull(Pe) == E_FIELDS_WITH_INVALID_VALUE){
		SetCurrentError(E_FIELDS_WITH_INVALID_VALUE);
		return E_INVALID_STRUCTURE;
	}
	/* check if the SizeOfOptionnalHeader field is correct */
	else if (PeFileHeaderHasGoodSizeOfOptionalHeader(Pe) == E_INVALID_SIZE_OPT_HEADER){
		SetCurrentError(E_INVALID_SIZE_OPT_HEADER);
		return E_INVALID_STRUCTURE;
	}
	/* check if the architecture DataDirectory, that should be null, is null */
	else if (PeHasDataDirectory(Pe, IMAGE_DIRECTORY_ENTRY_ARCHITECTURE) != E_NO_ENTRY){
		SetCurrentError(E_NO_ENTRY);
		return E_INVALID_STRUCTURE;
	}
	/* check if the globalPtr DataDirectory, that should be null, is null */
	else if (PeHasDataDirectory(Pe, IMAGE_DIRECTORY_ENTRY_GLOBALPTR) != E_NO_ENTRY){
		SetCurrentError(E_NO_ENTRY);
		return E_INVALID_STRUCTURE;
	}
	/* check if the last DataDirectory (actually its not a dataDirectory, but the last 8 bits
	of the header, and a DataDirectory is also 8 bits), that should be null, is null */
	else if (PeHasDataDirectory(Pe, IMAGE_DIRECTORY_ENTRY_END) != E_NO_ENTRY){
		SetCurrentError(E_NO_ENTRY);
		return E_INVALID_STRUCTURE;
	}
	/* check if the NumberOfRvaAndSizes field is correct */
	else if (PeGoodNumberOfRvaAndSizes(Pe) == E_INVALID_NUMBER_RVA_SIZES){
		SetCurrentError(E_INVALID_NUMBER_RVA_SIZES);
		return E_INVALID_STRUCTURE;
	}
	/* check if the FileAlignment field is correct */
	else if (PeHasValidSFAlignment(Pe) == E_INVALID_S_F_ALIGNMENT){
		SetCurrentError(E_INVALID_S_F_ALIGNMENT);
		return E_INVALID_STRUCTURE;
	}

	return ARMADITO_SUCCESS;
}

VOID PeDestroy(PPORTABLE_EXECUTABLE Pe) {
	free((QWORD*)Pe->BaseAddress);
}

/**
 * convert an RVA into the associated offset in the file
 * @param  Pe  the PORTABLE_EXECUTABLE representing the file
 * @param  Rva the RVA to convert
 * @return     a DWORD equal to the offset
 */
DWORD PeRvaToOffset(PPORTABLE_EXECUTABLE Pe, ULONG_PTR Rva) {
	DWORD i = 0;

	if (Rva == 0){
		return 0;
	}

	/* search for the section linked to the RVA */
	while (Pe->ImagesSectionHeader[i].VirtualAddress <= Rva &&  i < Pe->ImageFileHeader.NumberOfSections) i++;
	i--;

	/* check if i is between 0 and the number of sections (i is unsigned) */
	if (i >= Pe->ImageFileHeader.NumberOfSections){
		return Pe->FileSize + 2; /* this value cause the PeIsValidAddress to return FALSE */
	}

	return Pe->ImagesSectionHeader[i].PointerToRawData + ((DWORD)Rva - Pe->ImagesSectionHeader[i].VirtualAddress);
}

ERROR_CODE PeHasDataDirectory(PPORTABLE_EXECUTABLE Pe, DWORD imageDirectoryEntry){
	IMAGE_DATA_DIRECTORY entry;
	ULONG_PTR offset = 0;
	/* compute the offset of the wanted data directory depending on the machine */
	if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
		offset = (ULONG_PTR)FIELD_OFFSET32(IMAGE_NT_HEADERS32, OptionalHeader) + (ULONG_PTR)FIELD_OFFSET32(IMAGE_OPTIONAL_HEADER32, DataDirectory[imageDirectoryEntry]);
	}
	else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
		offset = (ULONG_PTR)FIELD_OFFSET64(IMAGE_NT_HEADERS64, OptionalHeader) + (ULONG_PTR)FIELD_OFFSET64(IMAGE_OPTIONAL_HEADER64, DataDirectory[imageDirectoryEntry]);
	}

	/* recovery of the entry : it must have a not null size and a not null virtual address */
	entry = *(PIMAGE_DATA_DIRECTORY)((PVOID)(Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew + offset));
	if (entry.Size == 0 || entry.VirtualAddress == 0){
		return E_NO_ENTRY;
	}
	else{
		return ARMADITO_SUCCESS;
	}
}

/**
 * give the offset associated to the wanted data directory entry
 * @param  Pe                  the PORTABLE_EXECUTABLE representing the file
 * @param  imageDirectoryEntry the wanted entry
 * @return                     a DWORD equal to the offset
 */
DWORD PeGetOffsetOfDataDirectory(PPORTABLE_EXECUTABLE Pe, DWORD imageDirectoryEntry) {
	IMAGE_DATA_DIRECTORY entry;
	DWORD retAddress = 0, i = 0;
	ULONG_PTR offset = 0;
	/* compute the offset of the wanted data directory depending on the machine */
	if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
		offset = (ULONG_PTR)FIELD_OFFSET32(IMAGE_NT_HEADERS32, OptionalHeader) + (ULONG_PTR)FIELD_OFFSET32(IMAGE_OPTIONAL_HEADER32, DataDirectory[imageDirectoryEntry]);
	}
	else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
		offset = (ULONG_PTR)FIELD_OFFSET64(IMAGE_NT_HEADERS64, OptionalHeader) + (ULONG_PTR)FIELD_OFFSET64(IMAGE_OPTIONAL_HEADER64, DataDirectory[imageDirectoryEntry]);
	}

	/* recovery of the entry and search for the associated section */
	entry = *(PIMAGE_DATA_DIRECTORY)((PVOID)(Pe->BaseAddress + Pe->ImageDosHeader.e_lfanew + offset));
	while (Pe->ImagesSectionHeader[i].VirtualAddress <= entry.VirtualAddress && i < Pe->ImageFileHeader.NumberOfSections){ i += 1; }
	i -= 1;

	if (i >= Pe->ImageFileHeader.NumberOfSections){
		return Pe->FileSize + 2; /* this value cause the PeIsValidAddress to return FALSE */
	}

	retAddress = Pe->ImagesSectionHeader[i].PointerToRawData + (entry.VirtualAddress - Pe->ImagesSectionHeader[i].VirtualAddress);

	return retAddress;
}

ERROR_CODE PeHasEntryPoint(PPORTABLE_EXECUTABLE Pe) {
	/* the entry point is in the OptionalHeader in the field AddressOfEntryPoint, and is not null is present */
	if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
		if (Pe->ImageFileHeader.SizeOfOptionalHeader != 0 && Pe->ImageOptionalHeader32.AddressOfEntryPoint){
			if (PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, Pe->ImageOptionalHeader32.AddressOfEntryPoint))){
				return ARMADITO_SUCCESS;
			}
			else{
				return E_INVALID_ENTRY_POINT;
			}
		}
		else{
			return E_NO_ENTRY_POINT;
		}
	}
	else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
		if (Pe->ImageFileHeader.SizeOfOptionalHeader != 0 && Pe->ImageOptionalHeader64.AddressOfEntryPoint){
			if (PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, Pe->ImageOptionalHeader64.AddressOfEntryPoint))){
				return ARMADITO_SUCCESS;
			}
			else{
				return E_INVALID_ENTRY_POINT;
			}
		}
		else{
			return E_NO_ENTRY_POINT;
		}
	}

	return E_NO_ENTRY_POINT;
}

TODO;
// faire en sorte de prende en compte le image->base pour les ordinaux, afin d'avoir les véritables ordinaux
// car pour le moment, deux ordinaux peuvent être égaux dans deux eat différents alors que leur base n'est pas la même
// ce qui fait que deux eat peuvent sembler identiques alors que non
/**
 * discussion about empty export tables : http://sourceware.org/ml/binutils/2008-08/msg00065.html
 */
ERROR_CODE GenerateExportedFunctions(PPORTABLE_EXECUTABLE Pe, PDATABASE_NODE DataBase, DWORD TotalSizeDataBase, PVECTOR* testFile) {
	QWORD *pQwVectors = NULL;
	QWORD qwNumberOfVector = 0;
	QWORD idNumberFromName = 0;
	DWORD indexQwV = 0;
	DWORD size = 0;
	PIMAGE_EXPORT_DIRECTORY Image = NULL;
	WORD i = 0;
	UCHAR* funcname = NULL;
	UCHAR* dllname = NULL;
	UCHAR* szBufferFinal = NULL;
	PDWORD addroffset = NULL;
	PDWORD addrval = NULL;
	WORD* addordinal = NULL;
	BOOLEAN* noNameFunctions = NULL;

	/* reading of the IMAGE_EXPORT_DIRECTORY containing the EAT */
	if (!PeIsValidAddress(Pe, Pe->BaseAddress + PeGetOffsetOfDataDirectory(Pe, IMAGE_DIRECTORY_ENTRY_EXPORT))){
		return (E_EAT_NOT_GOOD);
	}
	Image = (PIMAGE_EXPORT_DIRECTORY)((PVOID)(Pe->BaseAddress + PeGetOffsetOfDataDirectory(Pe, IMAGE_DIRECTORY_ENTRY_EXPORT)));
	if (!PeIsValidAddress(Pe, (ULONG_PTR)Image)){
		return (E_EAT_NOT_GOOD);
	}

	if (!IsValidTimeStamp(Image->TimeDateStamp)){
		return E_EAT_INVALID_TIMESTAMP;
	}

	/* check if the NumberOfFunctions is valid*/
	if ((DWORD)Image->NumberOfFunctions == 0){
		return (E_EAT_EMPTY);
	}
	if ((DWORD)Image->NumberOfFunctions > (WORD)-1 || (DWORD)Image->NumberOfFunctions < (DWORD)Image->NumberOfNames){
		return (E_EAT_NOT_GOOD);
	}

	/* creation of the array that vile contain the EAT */
	pQwVectors = (QWORD*)calloc((DWORD)Image->NumberOfFunctions, sizeof(QWORD));
	if (pQwVectors == NULL){
		return (E_CALLOC_ERROR);
	}

	/* valid offsets checking */
	if (!PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, Image->Name)) ||
		!PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, Image->AddressOfFunctions)) ||
		!PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, Image->AddressOfNames)) ||
		!PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, Image->AddressOfNameOrdinals))){
		free(pQwVectors);
		return (E_EAT_NOT_GOOD);
	}

	/* this array is used in order to know if a function has a name or not, in order to recover ordinal only functions */
	noNameFunctions = (BOOLEAN*)calloc((DWORD)Image->NumberOfFunctions, sizeof(BOOLEAN));
	if (noNameFunctions == NULL){
		free(pQwVectors);
		return (E_CALLOC_ERROR);
	}

	/* dll name recovery */
	dllname = (UCHAR*)((PVOID)(Pe->BaseAddress + PeRvaToOffset(Pe, Image->Name)));
	if (IsAValidDllName(dllname, strlen((CHAR*)dllname)) == FALSE){
		free(pQwVectors);
		free(noNameFunctions);
		return (E_DLL_NAME_ERROR);
	}
	ConvertToUpperCase((CHAR*)dllname);

	/**
	 * for each function with a name, the offsets of its name and its ordinal are computed
	 * the name is used in pQwVectors
	 * the ordinal is used in order to mark noNameFunctions[ordinal] as TRUE
	 */
	for (i = 0; i < Image->NumberOfNames; i++) {
		addroffset = (PDWORD)((PVOID)(Pe->BaseAddress + PeRvaToOffset(Pe, Image->AddressOfNames) + i*sizeof(DWORD)));
		addordinal = (WORD*)((PVOID)(Pe->BaseAddress + PeRvaToOffset(Pe, Image->AddressOfNameOrdinals) + i*sizeof(WORD)));

		/* the ordinal can't be higher than the number of functions */
		if (*addordinal >(DWORD)Image->NumberOfFunctions){
			free(pQwVectors);
			free(noNameFunctions);
			return (E_EAT_NOT_GOOD);
		}

		/* name offset checking and recovery */
		if (!PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, *addroffset))){
			free(pQwVectors);
			free(noNameFunctions);
			return (E_EAT_NOT_GOOD);
		}
		funcname = (UCHAR*)((PVOID)(Pe->BaseAddress + PeRvaToOffset(Pe, *addroffset)));
		if (IsAValidFunctionName(funcname, strlen((CHAR*)funcname)) == FALSE){
			free(pQwVectors);
			free(noNameFunctions);
			return (E_FUNCTION_NAME_ERROR);
		}

		/* two functions can't have the same ordinal */
		if (noNameFunctions[*addordinal] == TRUE){
			free(pQwVectors);
			free(noNameFunctions);
			return (E_EAT_NOT_GOOD);
		}
		noNameFunctions[*addordinal] = TRUE;

		/* creation of the CHAR* that will contain the couple dll/function */
		size = strlen((CHAR*)dllname) + strlen((CHAR*)funcname) + 2;
		szBufferFinal = (UCHAR *)calloc(strlen((CHAR*)dllname) + strlen((CHAR*)funcname) + 2, sizeof(UCHAR));
		if (szBufferFinal == NULL){
			free(pQwVectors);
			free(noNameFunctions);
			return (E_CALLOC_ERROR);
		}

		os_strncpy((CHAR*)szBufferFinal,size, (CHAR*)dllname, strlen((CHAR*)dllname));
		os_strncat((CHAR*)szBufferFinal,size, "!", 1);
		os_strncat((CHAR*)szBufferFinal,size, (CHAR*)funcname, strlen((CHAR*)funcname));

		/* recovery of the id of the couple from the database */
		/*pQwVectors[indexQwV] = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, strlen((CHAR*)dllname)+strlen((CHAR*)funcname)+1);
		indexQwV++;*/
		idNumberFromName = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, (DWORD)(strlen((CHAR*)dllname) + strlen((CHAR*)funcname) + 1));
		if (indexQwV < (DWORD)Image->NumberOfFunctions && idNumberFromName != (QWORD)-2){
			pQwVectors[indexQwV] = idNumberFromName;
			indexQwV++;
		}

		free(szBufferFinal);
	}

	/**
	 * for the rest of the functions (ie. functions exported by ordinal only), the address of each function is computed,
	 * then if it is not null, a virtual couple is created with the dll name and an ordinal equal to the current function
	 * (ie. if the current function is the i-th function, the ordinal is i)
	 * here, the ordinal value is based on the fact that functions are sorted by ordinal in the AddressOfFunctions array
	 * (fact base on the experiments)
	 */
	for (i = 0; i < Image->NumberOfFunctions; i++) {
		if (!noNameFunctions[i]){
			/* recovery of the address of the function */
			addrval = (PDWORD)((PVOID)(Pe->BaseAddress + PeRvaToOffset(Pe, Image->AddressOfFunctions) + i*sizeof(DWORD)));

			//if(!PeIsValidAddress(Pe, Pe->BaseAddress + *addrval)){
			//	free(pQwVectors);
			//	free(noNameFunctions);
			//	return (E_EAT_NOT_GOOD);
			//	//return 9876;
			//}

			if (*addrval){
				/* creation of the CHAR* that will contain the couple dll/ordinal */
				size = strlen((CHAR*)dllname) + strlen(ORDINAL_PREFIX_STRING_EXPORT) + 200;
				szBufferFinal = (UCHAR *)calloc(strlen((CHAR*)dllname) + strlen(ORDINAL_PREFIX_STRING_EXPORT) + 200, sizeof(UCHAR));
				if (szBufferFinal == NULL){
					free(pQwVectors);
					free(noNameFunctions);
					return (E_CALLOC_ERROR);
				}

				os_strncpy((CHAR*)szBufferFinal,size, (CHAR*)dllname, strlen((CHAR*)dllname));
				os_strncat((CHAR*)szBufferFinal,size, "!", 1);
				os_strncat((CHAR*)szBufferFinal,size, ORDINAL_PREFIX_STRING_EXPORT, strlen(ORDINAL_PREFIX_STRING_EXPORT));

#ifndef _MSC_VER
				snprintf(((CHAR*)szBufferFinal + strlen(ORDINAL_PREFIX_STRING_EXPORT) + strlen((CHAR*)dllname) + 1), \
					200 - strlen((CHAR*)dllname) - 1 - strlen(ORDINAL_PREFIX_STRING_EXPORT), \
					"%u", \
					i \
					);
#else
				sprintf_s(((CHAR*)szBufferFinal + strlen(ORDINAL_PREFIX_STRING_EXPORT) + strlen((CHAR*)dllname) + 1), \
					200 - strlen((CHAR*)dllname) - 1 - strlen(ORDINAL_PREFIX_STRING_EXPORT), \
					"%u", \
					i \
					);
#endif

				/* recovery of the id of the couple from the database */
				/*pQwVectors[indexQwV] = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, strlen((CHAR*)szBufferFinal));
				indexQwV++;*/
				idNumberFromName = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, (DWORD)strlen((CHAR*)szBufferFinal));
				if (indexQwV < (DWORD)Image->NumberOfFunctions &&idNumberFromName != (QWORD)-2){
					pQwVectors[indexQwV] = idNumberFromName;
					indexQwV++;
				}

				free(szBufferFinal);
			}
		}
	}
	free(noNameFunctions);

	/*  creation of the PVECTOR that will be used by all test functions */
	qwNumberOfVector = ((DWORD)indexQwV);
	if (qwNumberOfVector == 0){
		free(pQwVectors);
		return E_EMPTY_VECTOR;
	}
	BubbleSort(pQwVectors, qwNumberOfVector);
	*testFile = VectorCreateFromArray(pQwVectors, (DWORD)(qwNumberOfVector & 0xffffffff));

	if (*testFile == NULL){
		return E_CALLOC_ERROR;
	}

	return ARMADITO_SUCCESS;
}

/**
 * this function compute the number of functions in the IAT of a file
 * @param  Pe           the PORTABLE_EXECUTABLE representing the file
 * @param  CurrentImage the IAT of the file
 * @return              0 if there is an error, or no imported functions, or the number of imported functions
 */
DWORD PeGetNumberOfImportedFunctions(PPORTABLE_EXECUTABLE Pe, PIMAGE_IMPORT_DESCRIPTOR CurrentImage) {
	DWORD numberOfImportedFunctions = 0, offset = 0;
	PIMAGE_THUNK_DATA32 CurrentThunk = NULL;
	PIMAGE_THUNK_DATA64 CurrentThunk64 = NULL;
	BOOLEAN importFunctionsForCurrentDll = FALSE;

	/**
	 * the idea is simple : for each PIMAGE_IMPORT_DESCRIPTOR, the number of functions (thunk) inside it is read
	 * if it is 0, then 0 is return, on the other case, the following thunk is read
	 * when all the thunk are read, the function finish
	 */
	while (CurrentImage->OriginalFirstThunk != 0) {
		if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
			offset = PeRvaToOffset(Pe, CurrentImage->OriginalFirstThunk);
			if (!PeIsValidAddress(Pe, Pe->BaseAddress + offset)){
				return 0;
			}

			CurrentThunk = (PIMAGE_THUNK_DATA32)((PVOID)(Pe->BaseAddress + offset));

			while (CurrentThunk->u1.Function) {
				importFunctionsForCurrentDll = TRUE;
				numberOfImportedFunctions++;
				CurrentThunk++;
			}
		}
		else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
			offset = PeRvaToOffset(Pe, CurrentImage->OriginalFirstThunk);
			if (!PeIsValidAddress(Pe, Pe->BaseAddress + offset)){
				return 0;
			}
			CurrentThunk64 = (PIMAGE_THUNK_DATA64)((PVOID)(Pe->BaseAddress + offset));

			while (CurrentThunk64->u1.Function) {
				importFunctionsForCurrentDll = TRUE;
				numberOfImportedFunctions++;
				CurrentThunk64++;
			}
		}

		if (importFunctionsForCurrentDll == FALSE){
			return 0;
		}

		CurrentImage++;
		if (CurrentImage->OriginalFirstThunk == 0){
			CurrentImage->OriginalFirstThunk = CurrentImage->FirstThunk;
		}
	}

	return numberOfImportedFunctions;
}

ERROR_CODE GenerateImportedFunctions(PPORTABLE_EXECUTABLE Pe, PDATABASE_NODE DataBase, DWORD TotalSizeDataBase, PVECTOR* testFile) {
	DWORD iatOffset = 0;
	BOOLEAN importFunctionsForCurrentDll = FALSE; /* is set to TRUE when in a thunk, functions are imported */
	QWORD previousOrdinal = (QWORD)-1; /* ordinal initialization */
	// Some safe file have all their hint set to 0. Since this value is not forced to be set, the tst is
	// changed in order to allow iat that have all hint to 0 (and only 0)
	QWORD previousHint = (QWORD)-1; /* hint initialization */
	QWORD idNumberFromName = 0;
	UCHAR* dllname = NULL;
	PIMAGE_IMPORT_DESCRIPTOR CurrentImage = NULL;
	PIMAGE_IMPORT_BY_NAME CurrentFunction = NULL;
	PIMAGE_THUNK_DATA32 CurrentThunk = NULL;
	PIMAGE_THUNK_DATA64 CurrentThunk64 = NULL;
	QWORD qwNumberOfVector = 0;
	QWORD *pQwVectors = NULL;
	DWORD indexQwV = 0;
	DWORD size = 0;
	UCHAR* szBufferFinal = NULL;
	ULONG OrdinalNumber = 0;

	/* recovery of the offset of the IAT and the corresponding IMAGE_IMPORT_DESCRIPTOR */
	iatOffset = PeGetOffsetOfDataDirectory(Pe, IMAGE_DIRECTORY_ENTRY_IMPORT);
	if (iatOffset == Pe->FileSize + 2){
		return E_IAT_NOT_GOOD;
	}
	if (!PeIsValidAddress(Pe, Pe->BaseAddress + iatOffset)){
		return E_IAT_NOT_GOOD;
	}
	CurrentImage = (PIMAGE_IMPORT_DESCRIPTOR)((PVOID)(Pe->BaseAddress + iatOffset));

	/**
	 * sometimes, u1.OriginalFirstThunk must be set to FirstThunk : (from http://antiwpa.planet-dl.org/src/doc/EXE-%20ImportTable.htm)
	 * Check the value of u1.OriginalFirstThunk. If u1.OriginalFirstThunk is zero, use the value in FirstThunk instead.
	 * Some linkers generate PE files with 0 in u1.OriginalFirstThunk. This is considered a bug.
	 * Just to be on the safe side, we check the value in u1.OriginalFirstThunk first.
	 */
	if (CurrentImage->OriginalFirstThunk == 0){
		CurrentImage->OriginalFirstThunk = CurrentImage->FirstThunk;
	}
	if (CurrentImage->OriginalFirstThunk == 0){
		return E_IAT_NOT_GOOD;
	}

	if (CurrentImage->TimeDateStamp != (DWORD)-1 && !IsValidTimeStamp(CurrentImage->TimeDateStamp)){
		return E_IAT_INVALID_TIMESTAMP;
	}

	/* recovery of the number of imported functions, it will be the size of pQwVectors */
	qwNumberOfVector = PeGetNumberOfImportedFunctions(Pe, CurrentImage);
	if (qwNumberOfVector == 0){
		return E_IAT_EMPTY;
	}
	pQwVectors = (QWORD*)calloc(qwNumberOfVector, sizeof(QWORD));
	if (pQwVectors == NULL){
		return E_CALLOC_ERROR;
	}

	/**
	 * for each PIMAGE_IMPORT_DESCRIPTOR, the name of the dll represented by this PIMAGE_IMPORT_DESCRIPTOR is read
	 * then all the thunk inside this PIMAGE_IMPORT_DESCRIPTOR are read, and for each thunk, the associated function name (or ordinal) is read
	 * and used in order to create a dll/function (or dll/ordinal) couple
	 * comment about the ordinal test : (http://msdn.microsoft.com/en-us/library/aa278947%28v=vs.60%29.aspx)
	 * according to msdn : (about ordinals) they must be in the range 1 through N, where N is the number of functions exported by the DLL
	 * so this simply tests if a function has a different ordinal compared to the previous one
	 * the test also test the hint, as it is also supposed to be different for each functions (no source on this, just exports of common dll like kernel32)
	 */
	while (CurrentImage->OriginalFirstThunk != 0) {
		previousOrdinal = (QWORD)-1;
		previousHint = (QWORD)-1;
		importFunctionsForCurrentDll = FALSE;

		/* dll name reading */
		if (!PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, CurrentImage->Name))){
			free(pQwVectors);
			return E_IAT_NOT_GOOD;
		}
		dllname = (UCHAR*)((PVOID)(Pe->BaseAddress + PeRvaToOffset(Pe, CurrentImage->Name)));

		if (IsAValidDllName(dllname, strlen((CHAR*)dllname)) == FALSE){
			free(pQwVectors);
			return E_DLL_NAME_ERROR;
		}
		ConvertToUpperCase((CHAR*)dllname);

		/* the thunk structure depends on the architecture (x86 or x64) => a code for x86 and a code for x64 */
		if (Pe->Machine == IMAGE_FILE_MACHINE_I386){
			if (!PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, CurrentImage->OriginalFirstThunk))){
				free(pQwVectors);
				return E_IAT_NOT_GOOD;
			}
			CurrentThunk = (PIMAGE_THUNK_DATA32)((PVOID)(Pe->BaseAddress + PeRvaToOffset(Pe, CurrentImage->OriginalFirstThunk)));

			while (CurrentThunk->u1.Function) {
				importFunctionsForCurrentDll = TRUE;
				/* if the function IS NOT exported by ordinal only */
				if (!(CurrentThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32)){
					if (!PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, CurrentThunk->u1.AddressOfData))){
						free(pQwVectors);
						return E_IAT_NOT_GOOD;
					}
					CurrentFunction = (PIMAGE_IMPORT_BY_NAME)((PVOID)(Pe->BaseAddress + PeRvaToOffset(Pe, CurrentThunk->u1.AddressOfData)));

					if (IsAValidFunctionName((UCHAR*)CurrentFunction->Name, strlen((CHAR*)CurrentFunction->Name)) == FALSE){
						free(pQwVectors);
						return E_FUNCTION_NAME_ERROR;
					}

					/* testing of the hint of the function */
					if (CurrentFunction->Hint != 0 && previousHint == CurrentFunction->Hint){
						free(pQwVectors);
						return E_IAT_NOT_GOOD;
					}

					/* creation of the CHAR* that will contain the couple dll/ordinal */
					size = strlen((CHAR*)dllname) + strlen((CHAR*)CurrentFunction->Name) + 2;
					szBufferFinal = (UCHAR*)calloc(strlen((CHAR*)dllname) + strlen((CHAR*)CurrentFunction->Name) + 2, sizeof(UCHAR));
					if (szBufferFinal == NULL){
						free(pQwVectors);
						return E_CALLOC_ERROR;
					}
					os_strncpy((CHAR*)szBufferFinal,size, (CHAR*)dllname, strlen((CHAR*)dllname));
					os_strncat((CHAR*)szBufferFinal,size, "!", 1);
					os_strncat((CHAR*)szBufferFinal,size, (CHAR*)CurrentFunction->Name, strlen((CHAR*)CurrentFunction->Name));

					/*pQwVectors[indexQwV] = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, strlen((CHAR*)dllname)+strlen((CHAR*)CurrentFunction->Name)+1);
					indexQwV++;*/
					idNumberFromName = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, (DWORD)(strlen((CHAR*)dllname) + strlen((CHAR*)CurrentFunction->Name) + 1));
					if (indexQwV < qwNumberOfVector && idNumberFromName != (QWORD)-2){
						pQwVectors[indexQwV] = idNumberFromName;
						indexQwV++;
					}

					free(szBufferFinal);

					previousHint = CurrentFunction->Hint;
				}
				else{
					/* testing of the ordinal of the function */
					if (previousOrdinal == ((~IMAGE_ORDINAL_FLAG32) & CurrentThunk->u1.Ordinal)){
						free(pQwVectors);
						return E_IAT_NOT_GOOD;
					}

					OrdinalNumber = ((~IMAGE_ORDINAL_FLAG32) & CurrentThunk->u1.Ordinal);
					/* creation of the CHAR* that will contain the couple dll/ordinal */
					size = strlen((CHAR*)dllname) + strlen(ORDINAL_PREFIX_STRING_IMPORT) + 200;
					szBufferFinal = (UCHAR*)calloc(strlen((CHAR*)dllname) + strlen(ORDINAL_PREFIX_STRING_IMPORT) + 200, sizeof(UCHAR));
					if (szBufferFinal == NULL){
						free(pQwVectors);
						return E_IAT_NOT_GOOD;
					}
					os_strncpy((CHAR*)szBufferFinal,size, (CHAR*)dllname, strlen((CHAR*)dllname));
					os_strncat((CHAR*)szBufferFinal,size, "!", 1);
					os_strncat((CHAR*)szBufferFinal,size, ORDINAL_PREFIX_STRING_IMPORT, strlen(ORDINAL_PREFIX_STRING_IMPORT));

#ifndef _MSC_VER
					snprintf((CHAR*)(szBufferFinal + strlen(ORDINAL_PREFIX_STRING_IMPORT) + strlen((CHAR*)dllname) + 1), \
						200 - strlen((CHAR*)dllname) - 1 - strlen(ORDINAL_PREFIX_STRING_IMPORT), \
						"%lu", \
						OrdinalNumber \
						);
#else
					sprintf_s((CHAR*)(szBufferFinal + strlen(ORDINAL_PREFIX_STRING_IMPORT) + strlen((CHAR*)dllname) + 1), \
						200 - strlen((CHAR*)dllname) - 1 - strlen(ORDINAL_PREFIX_STRING_IMPORT), \
						"%lu", \
						OrdinalNumber \
						);
#endif

					/*pQwVectors[indexQwV] = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, strlen((CHAR*)szBufferFinal));
					indexQwV++;*/
					idNumberFromName = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, (DWORD)strlen((CHAR*)szBufferFinal));
					if (indexQwV < qwNumberOfVector && idNumberFromName != (QWORD)-2){
						pQwVectors[indexQwV] = idNumberFromName;
						indexQwV++;
					}

					free(szBufferFinal);

					previousOrdinal = OrdinalNumber;
				}

				CurrentThunk++;
			}
		}
		else if (Pe->Machine == IMAGE_FILE_MACHINE_AMD64){
			if (!PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, CurrentImage->OriginalFirstThunk))){
				free(pQwVectors);
				return E_IAT_NOT_GOOD;
			}
			CurrentThunk64 = (PIMAGE_THUNK_DATA64)((PVOID)(Pe->BaseAddress + PeRvaToOffset(Pe, CurrentImage->OriginalFirstThunk)));

			while (CurrentThunk64->u1.Function) {
				importFunctionsForCurrentDll = TRUE;
				/* if the function IS NOT exported by ordinal only */
				if (!(CurrentThunk64->u1.Ordinal & IMAGE_ORDINAL_FLAG64)){
					if (!PeIsValidAddress(Pe, Pe->BaseAddress + PeRvaToOffset(Pe, CurrentThunk64->u1.AddressOfData))){
						free(pQwVectors);
						return E_IAT_NOT_GOOD;
					}
					CurrentFunction = (PIMAGE_IMPORT_BY_NAME)((PVOID)(Pe->BaseAddress + PeRvaToOffset(Pe, CurrentThunk64->u1.AddressOfData)));

					if (IsAValidFunctionName((UCHAR*)CurrentFunction->Name, strlen((CHAR*)CurrentFunction->Name)) == FALSE){
						free(pQwVectors);
						return E_FUNCTION_NAME_ERROR;
					}

					/* testing of the hint of the function */
					if (CurrentFunction->Hint != 0 && previousHint == CurrentFunction->Hint){
						free(pQwVectors);
						return E_IAT_NOT_GOOD;
					}

					/* creation of the CHAR* that will contain the couple dll/ordinal */
					size = strlen((CHAR*)dllname) + strlen((CHAR*)CurrentFunction->Name) + 2 ;
					szBufferFinal = (UCHAR*)calloc(strlen((CHAR*)dllname) + strlen((CHAR*)CurrentFunction->Name) + 2, sizeof(UCHAR));
					if (szBufferFinal == NULL){
						free(pQwVectors);
						return E_CALLOC_ERROR;
					}
					os_strncpy((CHAR*)szBufferFinal,size, (CHAR*)dllname, strlen((CHAR*)dllname));
					os_strncat((CHAR*)szBufferFinal,size, "!", 1);
					os_strncat((CHAR*)szBufferFinal,size, (CHAR*)CurrentFunction->Name, strlen((CHAR*)CurrentFunction->Name));

					/*pQwVectors[indexQwV] = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, strlen((CHAR*)dllname)+strlen((CHAR*)CurrentFunction->Name)+1);
					indexQwV++;*/
					idNumberFromName = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, (DWORD)(strlen((CHAR*)dllname) + strlen((CHAR*)CurrentFunction->Name) + 1));
					if (indexQwV < qwNumberOfVector && idNumberFromName != (QWORD)-2){
						pQwVectors[indexQwV] = idNumberFromName;
						indexQwV++;
					}

					free(szBufferFinal);

					previousHint = CurrentFunction->Hint;
				}
				else{
					/* testing of the ordinal of the function */
					if (previousOrdinal == ((~IMAGE_ORDINAL_FLAG64) & CurrentThunk64->u1.Ordinal)){
						free(pQwVectors);
						return E_IAT_NOT_GOOD;
					}

					OrdinalNumber = (ULONG)((~IMAGE_ORDINAL_FLAG64) & CurrentThunk64->u1.Ordinal);

					/* creation of the CHAR* that will contain the couple dll/ordinal */
					size = strlen((CHAR*)dllname) + strlen(ORDINAL_PREFIX_STRING_IMPORT) + 200;
					szBufferFinal = (UCHAR*)calloc(strlen((CHAR*)dllname) + strlen(ORDINAL_PREFIX_STRING_IMPORT) + 200, sizeof(UCHAR));
					if (szBufferFinal == NULL){
						free(pQwVectors);
						return E_IAT_NOT_GOOD;
					}
					os_strncpy((CHAR*)szBufferFinal,size, (CHAR*)dllname, strlen((CHAR*)dllname));
					os_strncat((CHAR*)szBufferFinal,size, "!", 1);
					os_strncat((CHAR*)szBufferFinal,size, ORDINAL_PREFIX_STRING_IMPORT, strlen(ORDINAL_PREFIX_STRING_IMPORT));
#ifndef _MSC_VER
					snprintf((CHAR*)(szBufferFinal + strlen(ORDINAL_PREFIX_STRING_IMPORT) + strlen((CHAR*)dllname) + 1), \
						200 - strlen((CHAR*)dllname) - 1 - strlen(ORDINAL_PREFIX_STRING_IMPORT), \
						"%lu", \
						OrdinalNumber \
						);
#else
					sprintf_s((CHAR*)(szBufferFinal + strlen(ORDINAL_PREFIX_STRING_IMPORT) + strlen((CHAR*)dllname) + 1), \
						200 - strlen((CHAR*)dllname) - 1 - strlen(ORDINAL_PREFIX_STRING_IMPORT), \
						"%lu", \
						OrdinalNumber \
						);
#endif

					/*pQwVectors[indexQwV] = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, strlen((CHAR*)szBufferFinal));
					indexQwV++;*/
					idNumberFromName = GetIdNumberFromName(DataBase, TotalSizeDataBase, szBufferFinal, (DWORD)strlen((CHAR*)szBufferFinal));
					if (indexQwV < qwNumberOfVector && idNumberFromName != (QWORD)-2){
						pQwVectors[indexQwV] = idNumberFromName;
						indexQwV++;
					}

					free(szBufferFinal);

					previousOrdinal = OrdinalNumber;
				}

				CurrentThunk64++;
			}
		}

		/* tests if in the current dll, imported functions were seen */
		if (importFunctionsForCurrentDll == FALSE){
			free(pQwVectors);
			return E_IAT_NOT_GOOD;
		}

		CurrentImage++;
		if (CurrentImage->OriginalFirstThunk == 0){
			CurrentImage->OriginalFirstThunk = CurrentImage->FirstThunk;
		}
		if (CurrentImage->TimeDateStamp != (DWORD)-1 && !IsValidTimeStamp(CurrentImage->TimeDateStamp)){
			free(pQwVectors);
			return E_IAT_INVALID_TIMESTAMP;
		}
	}

	/*  creation of the PVECTOR that will be used by all test functions */
	qwNumberOfVector = ((DWORD)indexQwV);
	if (qwNumberOfVector == 0){
		free(pQwVectors);
		return E_EMPTY_VECTOR;
	}

	BubbleSort(pQwVectors, qwNumberOfVector);
	*testFile = VectorCreateFromArray(pQwVectors, (DWORD)(qwNumberOfVector & 0xffffffff));

	if (*testFile == NULL){
		return E_CALLOC_ERROR;
	}

	return ARMADITO_SUCCESS;
}
