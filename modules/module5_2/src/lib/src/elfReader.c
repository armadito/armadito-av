#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "elfReader.h"
#include "utils.h"
#include "osdeps.h"

#define		TRUE				1
#define		FALSE				0

/**
 * Check if a specific offset is good toward a specific file with ELF format
 * @param  elfOfFile in: a pointer to the ELF_CONTAINER representing the file
 * @param  offset    in: the offset to be testes
 * @return           TRUE is the offset is good, FALSE otherwise
 */
BYTE ElfIsAValidOffset(PELF_CONTAINER elfOfFile, QWORD offset){
	if (offset < elfOfFile->fileSize){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

ERROR_CODE ElfInit(int fd, CHAR* filename, PELF_CONTAINER elfOfFile){
	FILE* fileHandle = NULL;
	WORD i = 0;
	DWORD signature = 0;

	fileHandle = os_fdopen(fd, "r");
	if (fileHandle == NULL){
		return E_FILE_NOT_FOUND;
	}

	/* computation of the file size */
	elfOfFile->fileSize = SizeOfFile(filename);
	if (elfOfFile->fileSize == 0){
		fclose(fileHandle);
		return E_FILE_EMPTY;
	}

	/* creation of the ULONG_PTR used to store the file */
	elfOfFile->buffer = (ULONG_PTR)calloc(elfOfFile->fileSize + 1, sizeof(UCHAR));
	if (elfOfFile->buffer == 0){
		fclose(fileHandle);
		return E_CALLOC_ERROR;
	}

	/* file reading into *elf 
		We read the whole file.
	*/
	if (fread((PVOID)elfOfFile->buffer, elfOfFile->fileSize, 1, fileHandle) != 1){
		fclose(fileHandle);
		free((ULONG_PTR*)elfOfFile->buffer);
		elfOfFile->buffer = 0;
		return E_READING_ERROR;
	}

	fclose(fileHandle);

	signature = *(DWORD*)((PVOID)(elfOfFile->buffer));

	if (signature != ELF_SIG){
		free((ULONG_PTR*)elfOfFile->buffer);
		elfOfFile->buffer = 0;
		return E_NOT_ELF;
	}
 
	// we parse e_machine value in header
	if (ElfIsAValidOffset(elfOfFile, EI_NIDENT * sizeof(UCHAR) + sizeof(WORD)) == TRUE){
		elfOfFile->machine = *(WORD*)((PVOID)(elfOfFile->buffer + EI_NIDENT * sizeof(UCHAR) + sizeof(WORD)));
	}
	else{
		free((ULONG_PTR*)elfOfFile->buffer);
		elfOfFile->buffer = 0;
		return E_BAD_FORMAT;
	}


	// ELF-64bit
	if (0 != elfOfFile->machine && EM_AMD64 == elfOfFile->machine){
		elfOfFile->PElfN_Ehdr.ehdr64 = (PElf64_Ehdr)((PVOID)(elfOfFile->buffer));
		elfOfFile->PElfN_Shdr.shdr64 = (PElf64_Shdr)calloc(elfOfFile->PElfN_Ehdr.ehdr64->e_shnum, sizeof(Elf64_Shdr));
		if (elfOfFile->PElfN_Shdr.shdr64 == NULL){
			free((ULONG_PTR*)elfOfFile->buffer);
			elfOfFile->buffer = 0;
			return E_CALLOC_ERROR;
		}

		// We parse all section headers
		for (i = 0; i < elfOfFile->PElfN_Ehdr.ehdr64->e_shnum; i++) {

			if (ElfIsAValidOffset(elfOfFile, elfOfFile->PElfN_Ehdr.ehdr64->e_shoff + elfOfFile->PElfN_Ehdr.ehdr64->e_shentsize*i) == TRUE){
				elfOfFile->PElfN_Shdr.shdr64[i] = *(Elf64_Shdr*)((PVOID)(elfOfFile->buffer + elfOfFile->PElfN_Ehdr.ehdr64->e_shoff + elfOfFile->PElfN_Ehdr.ehdr64->e_shentsize*i));
			}
			else{
				ElfDestroy(elfOfFile);
				return E_BAD_FORMAT;
			}
		}

	}  // ELF-32Bit
	else if (0 != elfOfFile->machine && EM_386 == elfOfFile->machine){ 
		elfOfFile->PElfN_Ehdr.ehdr32 = (PElf32_Ehdr)((PVOID)(elfOfFile->buffer));
		elfOfFile->PElfN_Shdr.shdr32 = (PElf32_Shdr)calloc(elfOfFile->PElfN_Ehdr.ehdr32->e_shnum, sizeof(Elf32_Shdr));
		if (elfOfFile->PElfN_Shdr.shdr32 == NULL){
			free((ULONG_PTR*)elfOfFile->buffer);
			elfOfFile->buffer = 0;
			return E_CALLOC_ERROR;
		}

		// We parse all section headers
		for (i = 0; i < elfOfFile->PElfN_Ehdr.ehdr32->e_shnum; i++) {
			if (ElfIsAValidOffset(elfOfFile, elfOfFile->PElfN_Ehdr.ehdr32->e_shoff + elfOfFile->PElfN_Ehdr.ehdr32->e_shentsize*i) == TRUE){
				elfOfFile->PElfN_Shdr.shdr32[i] = *(Elf32_Shdr*)((PVOID)(elfOfFile->buffer + elfOfFile->PElfN_Ehdr.ehdr32->e_shoff + elfOfFile->PElfN_Ehdr.ehdr32->e_shentsize*i));
			}
			else{
				ElfDestroy(elfOfFile);
				return E_BAD_FORMAT;
			}
		}
	}
	else{ // We check only IntelX86 and AMD64
		DBG_PRNT(" BAD_ARCH : %d ", elfOfFile->machine);
		free((ULONG_PTR*)elfOfFile->buffer);
		elfOfFile->buffer = 0;
		return E_BAD_ARCHITECTURE;
	}

	return UH_SUCCESS;
}

VOID ElfDestroy(PELF_CONTAINER elfOfFile){
	free((ULONG_PTR*)elfOfFile->buffer);
	elfOfFile->buffer = 0;
	if (0 != elfOfFile->machine && EM_AMD64 == elfOfFile->machine){
		free(elfOfFile->PElfN_Shdr.shdr64);
		elfOfFile->PElfN_Shdr.shdr64 = NULL;
	}
	else if (0 != elfOfFile->machine && EM_386 == elfOfFile->machine){
		free(elfOfFile->PElfN_Shdr.shdr32);
		elfOfFile->PElfN_Shdr.shdr32 = NULL;
	}
}

/**
 * this function compute the number of symbols imported
 * @param  elfOfFile    a pointer on the ELF_CONTAINER representing the file
 * @return              0 if there is an error, or no imported symbols;
 *                      or the number of imported symbols
 */
QWORD ElfNumberOfImportedSymbols(PELF_CONTAINER elfOfFile){
	WORD i = 0;
	QWORD qwNumberOfImportedSymbols = 0;

	if (0 != elfOfFile->machine && EM_AMD64 == elfOfFile->machine){
		for (i = 0; i < elfOfFile->PElfN_Ehdr.ehdr64->e_shnum; i++) {
			if ((elfOfFile->PElfN_Shdr.shdr64[i].sh_type == SHT_SYMTAB) || (elfOfFile->PElfN_Shdr.shdr64[i].sh_type == SHT_DYNSYM)) {
				qwNumberOfImportedSymbols += (elfOfFile->PElfN_Shdr.shdr64[i].sh_size / sizeof(Elf64_Sym));
			}
		}
	}
	else if (0 != elfOfFile->machine && EM_386 == elfOfFile->machine){
		for (i = 0; i < elfOfFile->PElfN_Ehdr.ehdr32->e_shnum; i++) {
			if ((elfOfFile->PElfN_Shdr.shdr32[i].sh_type == SHT_SYMTAB) || (elfOfFile->PElfN_Shdr.shdr32[i].sh_type == SHT_DYNSYM)){
				qwNumberOfImportedSymbols += (elfOfFile->PElfN_Shdr.shdr32[i].sh_size / sizeof(Elf32_Sym));
			}
		}
	}

	return qwNumberOfImportedSymbols;
}

/**
 * read the content of a symbols section, and store it into a QWORD array
 * @param  elfOfFile              a pointer on an ELF_CONTAINER structure
 * @param  sectionIndex           the index of the section to read in the section table
 * @param  pqwSymbols             the QWORD* where the symbols are stores
 * @param  currentPqwSymbolsIndex a pointer on a QWORD representing the first empty position in the array. if n symbols are
 *                                added, the valu will be increase by n
 * @param  db                     the PDATABASE_NODE representing the database
 * @param  db_size                the size of the database
 * @return                        E_BAD_FORMAT 		 if the file contains errors,
 *                                E_BAD_ARCHITECTURE if the file is neither an i386 file nor an amd64 file,
 *                                E_SUCCES  		 if everything went right
 */
ERROR_CODE ElfReadSymbolTable(PELF_CONTAINER elfOfFile, WORD sectionIndex, QWORD* *pqwSymbols, QWORD *currentPqwSymbolsIndex, PDATABASE_NODE db, DWORD db_size){
	UCHAR* str_tbl = NULL;
	PElf32_Sym sym_tbl32 = NULL;
	PElf64_Sym sym_tbl64 = NULL;
	DWORD i = 0, symbol_count = 0, str_tbl_ndx = 0, j = 0;
	UCHAR* tmpSymbolName = NULL;
	QWORD tmpIdNumber = 0;
	BYTE alreadySeenSymbol = FALSE;

	if (0 != elfOfFile->machine && EM_AMD64 == elfOfFile->machine){
		if (ElfIsAValidOffset(elfOfFile, elfOfFile->PElfN_Shdr.shdr64[sectionIndex].sh_offset) == TRUE){
			sym_tbl64 = (PElf64_Sym)((PVOID)(elfOfFile->buffer + elfOfFile->PElfN_Shdr.shdr64[sectionIndex].sh_offset));
		}
		else{
			return E_BAD_FORMAT;
		}

		str_tbl_ndx = elfOfFile->PElfN_Shdr.shdr64[sectionIndex].sh_link;
		if (ElfIsAValidOffset(elfOfFile, elfOfFile->PElfN_Shdr.shdr64[str_tbl_ndx].sh_offset) == TRUE){
			str_tbl = (UCHAR*)((PVOID)(elfOfFile->buffer + elfOfFile->PElfN_Shdr.shdr64[str_tbl_ndx].sh_offset));
		}
		else{
			return E_BAD_FORMAT;
		}

		symbol_count = (DWORD)(elfOfFile->PElfN_Shdr.shdr64[sectionIndex].sh_size / sizeof(Elf64_Sym));

		for (i = 0; i < symbol_count; i++) {
			alreadySeenSymbol = FALSE;
			if (ELF64_ST_TYPE(sym_tbl64[i].st_info) == STT_FUNC){// if the symbol has the function type
				if (ElfIsAValidOffset(elfOfFile, elfOfFile->PElfN_Shdr.shdr64[str_tbl_ndx].sh_offset + sym_tbl64[i].st_name) == TRUE){
					tmpSymbolName = str_tbl + sym_tbl64[i].st_name;
				}
				else{
					return E_BAD_FORMAT;
				}
				if (tmpSymbolName != NULL && strlen((CHAR*)tmpSymbolName) != 0){
					tmpIdNumber = GetIdNumberFromName(db, db_size, tmpSymbolName, (DWORD)strlen((CHAR*)tmpSymbolName));

					if (tmpIdNumber == (QWORD)-2){
						/*printf("%d:[%s]\n", tmpIdNumber, tmpSymbolName);*/
						/*
											*currentPqwSymbolsIndex += 1;
											(*pqwSymbols)[*currentPqwSymbolsIndex] = tmpIdNumber;
											*/
					}
					else{
						for (j = 0; j < *currentPqwSymbolsIndex; j++){
							if (tmpIdNumber == (*pqwSymbols)[j]){
								alreadySeenSymbol = TRUE;
							}
						}
						if (alreadySeenSymbol == FALSE){
							/*printf("%7d [%2d]:[%s]\n", tmpIdNumber, ELF32_ST_TYPE(sym_tbl64[i].st_info), tmpSymbolName);*/

							*currentPqwSymbolsIndex += 1;
							(*pqwSymbols)[*currentPqwSymbolsIndex] = tmpIdNumber;
						}
					}
				}
			}
		}
	}
	else if (0 != elfOfFile->machine && EM_386 == elfOfFile->machine){
		if (ElfIsAValidOffset(elfOfFile, elfOfFile->PElfN_Shdr.shdr32[sectionIndex].sh_offset) == TRUE){
			sym_tbl32 = (PElf32_Sym)((PVOID)(elfOfFile->buffer + elfOfFile->PElfN_Shdr.shdr32[sectionIndex].sh_offset));
		}
		else{
			return E_BAD_FORMAT;
		}

		str_tbl_ndx = elfOfFile->PElfN_Shdr.shdr32[sectionIndex].sh_link;
		if (ElfIsAValidOffset(elfOfFile, elfOfFile->PElfN_Shdr.shdr32[str_tbl_ndx].sh_offset) == TRUE){
			str_tbl = (UCHAR*)((PVOID)(elfOfFile->buffer + elfOfFile->PElfN_Shdr.shdr32[str_tbl_ndx].sh_offset));
		}
		else{
			return E_BAD_FORMAT;
		}

		symbol_count = (elfOfFile->PElfN_Shdr.shdr32[sectionIndex].sh_size / sizeof(Elf32_Sym));

		for (i = 0; i < symbol_count; i++) {
			if (ELF32_ST_TYPE(sym_tbl32[i].st_info) == STT_FUNC){// if the symbol has the function type
				alreadySeenSymbol = FALSE;
				if (ElfIsAValidOffset(elfOfFile, elfOfFile->PElfN_Shdr.shdr32[str_tbl_ndx].sh_offset + sym_tbl32[i].st_name) == TRUE){
					tmpSymbolName = str_tbl + sym_tbl32[i].st_name;
				}
				else{
					return E_BAD_FORMAT;
				}
				if (tmpSymbolName != NULL && strlen((CHAR*)tmpSymbolName) != 0){
					tmpIdNumber = GetIdNumberFromName(db, db_size, tmpSymbolName, (DWORD)strlen((CHAR*)tmpSymbolName));

					if (tmpIdNumber == (QWORD)-2){
						/*printf("%d:[%s]\n", tmpIdNumber, tmpSymbolName);*/
						/*
											*currentPqwSymbolsIndex += 1;
											(*pqwSymbols)[*currentPqwSymbolsIndex] = tmpIdNumber;
											*/
					}
					else{
						for (j = 0; j < *currentPqwSymbolsIndex; j++){
							if (tmpIdNumber == (*pqwSymbols)[j]){
								alreadySeenSymbol = TRUE;
							}
						}
						if (alreadySeenSymbol == FALSE){
							/*printf("%d:[%s]\n", tmpIdNumber, tmpSymbolName);*/

							*currentPqwSymbolsIndex += 1;
							(*pqwSymbols)[*currentPqwSymbolsIndex] = tmpIdNumber;
						}
					}
				}
			}
		}
	}
	else{
		return E_BAD_ARCHITECTURE;
	}

	return UH_SUCCESS;
}

/**
 * sort a QWORD array with the bubble sort algorithm
 * @param  t the array to be sorted
 * @param  n the size of the array
 */
VOID ElfBubbleSort(QWORD *t, QWORD n){
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

ERROR_CODE ElfSymbolTable(PELF_CONTAINER elfOfFile, PVECTOR *symbolVector, PDATABASE_NODE db, DWORD db_size){
	WORD i = 0;
	QWORD currentPqwSymbolsIndex = 0;
	QWORD qwNumberOfImportedSymbols = 0;
	QWORD* pqwSymbols = NULL;

	qwNumberOfImportedSymbols = ElfNumberOfImportedSymbols(elfOfFile);

	if (qwNumberOfImportedSymbols == 0){
		return E_SYMBOL_TABLE_EMPTY;
	}

	pqwSymbols = (QWORD*)calloc(qwNumberOfImportedSymbols, sizeof(QWORD));
	if (pqwSymbols == NULL){
		return E_CALLOC_ERROR;
	}

	if (0 != elfOfFile->machine && EM_AMD64 == elfOfFile->machine){
		for (i = 0; i < elfOfFile->PElfN_Ehdr.ehdr64->e_shnum; i++) {
			if (elfOfFile->PElfN_Shdr.shdr64[i].sh_name < elfOfFile->fileSize){
				if ((elfOfFile->PElfN_Shdr.shdr64[i].sh_type == SHT_SYMTAB) || (elfOfFile->PElfN_Shdr.shdr64[i].sh_type == SHT_DYNSYM)) {
					if (ElfReadSymbolTable(elfOfFile, i, &pqwSymbols, &currentPqwSymbolsIndex, db, db_size) != UH_SUCCESS){
						free(pqwSymbols);
						pqwSymbols = NULL;
						return E_BAD_FORMAT;
					}
				}
			}
		}
	}
	else if (0 != elfOfFile->machine && EM_386 == elfOfFile->machine){
		for (i = 0; i < elfOfFile->PElfN_Ehdr.ehdr32->e_shnum; i++) {
			if (elfOfFile->PElfN_Shdr.shdr32[i].sh_name < elfOfFile->fileSize){
				if ((elfOfFile->PElfN_Shdr.shdr32[i].sh_type == SHT_SYMTAB) || (elfOfFile->PElfN_Shdr.shdr32[i].sh_type == SHT_DYNSYM)) {
					if (ElfReadSymbolTable(elfOfFile, i, &pqwSymbols, &currentPqwSymbolsIndex, db, db_size) != UH_SUCCESS){
						free(pqwSymbols);
						pqwSymbols = NULL;
						return E_BAD_FORMAT;
					}
				}
			}
		}
	}
	else{
		free(pqwSymbols);
		pqwSymbols = NULL;
		return E_BAD_ARCHITECTURE;
	}

	if (currentPqwSymbolsIndex == 0){
		free(pqwSymbols);
		pqwSymbols = NULL;
		return E_NO_KNOWN_SYMBOLS;
	}

	ElfBubbleSort(pqwSymbols, currentPqwSymbolsIndex);

	*symbolVector = VectorCreateFromArray(pqwSymbols, (DWORD)(currentPqwSymbolsIndex & 0xffffffff));

	if (*symbolVector == NULL){
		return E_CALLOC_ERROR;
	}

	return UH_SUCCESS;
}
