/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito module H1.

Armadito module H1 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito module H1 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito module H1.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef ELF_READER_H
#define ELF_READER_H

#include <stdint.h>

#include "windowsTypes.h"
#include "h1-errors.h"
#include "windowsTypes.h"
#include "databases.h"
#include "vector.h"

/* x86 and x64 types */
#define 	Elf32_Addr 		uint32_t
#define 	Elf32_Off 		uint32_t
#define 	Elf64_Addr 		uint64_t
#define 	Elf64_Off 		uint64_t

/* unchanged types */
#define 	Elf32_Section 	uint16_t
#define 	Elf32_Versym 	uint16_t
#define 	Elf32_Half 		uint16_t
#define 	Elf32_Sword 	int32_t
#define 	Elf32_Word 		uint32_t
#define 	Elf32_Sxword 	int64_t
#define 	Elf32_Xword 	uint64_t
#define 	Elf64_Section 	uint16_t
#define 	Elf64_Versym 	uint16_t
#define 	Elf64_Half 		uint16_t
#define 	Elf64_Sword 	int32_t
#define 	Elf64_Word 		uint32_t
#define 	Elf64_Sxword 	int64_t
#define 	Elf64_Xword 	uint64_t

#define 	STT_FUNC 		2
#define 	ELF64_ST_TYPE(i)   ((i)&0xf)
#define 	ELF32_ST_TYPE(i)   ((i)&0xf)

#define 	EI_NIDENT 		16

#define		SHT_SYMTAB		2
#define		SHT_DYNSYM		11

#define		EM_AMD64		62		/* AMDs x86-64 architecture */
#define		EM_386			3		/* Intel 80386 */

#define 	ELF_SIG 		0x464C457F

typedef struct  _Elf32_Ehdr {
	unsigned char	e_ident[EI_NIDENT];	/* ident bytes */
	Elf32_Half	e_type;			/* file type */
	Elf32_Half	e_machine;		/* target machine */
	Elf32_Word	e_version;		/* file version */
	Elf32_Addr	e_entry;		/* start address */
	Elf32_Off	e_phoff;		/* phdr file offset */
	Elf32_Off	e_shoff;		/* shdr file offset */
	Elf32_Word	e_flags;		/* file flags */
	Elf32_Half	e_ehsize;		/* sizeof ehdr */
	Elf32_Half	e_phentsize;		/* sizeof phdr */
	Elf32_Half	e_phnum;		/* number phdrs */
	Elf32_Half	e_shentsize;		/* sizeof shdr */
	Elf32_Half	e_shnum;		/* number shdrs */
	Elf32_Half	e_shstrndx;		/* shdr string index */
} Elf32_Ehdr, *PElf32_Ehdr;

typedef struct  _Elf64_Ehdr {
	unsigned char	e_ident[EI_NIDENT];	/* ident bytes */
	Elf64_Half	e_type;			/* file type */
	Elf64_Half	e_machine;		/* target machine */
	Elf64_Word	e_version;		/* file version */
	Elf64_Addr	e_entry;		/* start address */
	Elf64_Off	e_phoff;		/* phdr file offset */
	Elf64_Off	e_shoff;		/* shdr file offset */
	Elf64_Word	e_flags;		/* file flags */
	Elf64_Half	e_ehsize;		/* sizeof ehdr */
	Elf64_Half	e_phentsize;		/* sizeof phdr */
	Elf64_Half	e_phnum;		/* number phdrs */
	Elf64_Half	e_shentsize;		/* sizeof shdr */
	Elf64_Half	e_shnum;		/* number shdrs */
	Elf64_Half	e_shstrndx;		/* shdr string index */
} Elf64_Ehdr, *PElf64_Ehdr;

typedef struct {
	Elf32_Word	sh_name;	/* section name */
	Elf32_Word	sh_type;	/* SHT_... */
	Elf32_Word	sh_flags;	/* SHF_... */
	Elf32_Addr	sh_addr;	/* virtual address */
	Elf32_Off	sh_offset;	/* file offset */
	Elf32_Word	sh_size;	/* section size */
	Elf32_Word	sh_link;	/* misc info */
	Elf32_Word	sh_info;	/* misc info */
	Elf32_Word	sh_addralign;	/* memory alignment */
	Elf32_Word	sh_entsize;	/* entry size if table */
} Elf32_Shdr, *PElf32_Shdr;

typedef struct {
	Elf64_Word	sh_name;	/* section name */
	Elf64_Word	sh_type;	/* SHT_... */
	Elf64_Xword	sh_flags;	/* SHF_... */
	Elf64_Addr	sh_addr;	/* virtual address */
	Elf64_Off	sh_offset;	/* file offset */
	Elf64_Xword	sh_size;	/* section size */
	Elf64_Word	sh_link;	/* misc info */
	Elf64_Word	sh_info;	/* misc info */
	Elf64_Xword	sh_addralign;	/* memory alignment */
	Elf64_Xword	sh_entsize;	/* entry size if table */
} Elf64_Shdr, *PElf64_Shdr;

typedef struct {
	Elf32_Word	st_name;
	Elf32_Addr	st_value;
	Elf32_Word	st_size;
	unsigned char	st_info;	/* bind, type: ELF_32_ST_... */
	unsigned char	st_other;
	Elf32_Half	st_shndx;	/* SHN_... */
} Elf32_Sym, *PElf32_Sym;

typedef struct {
	Elf64_Word	st_name;
	unsigned char	st_info;	/* bind, type: ELF_64_ST_... */
	unsigned char	st_other;
	Elf64_Half	st_shndx;	/* SHN_... */
	Elf64_Addr	st_value;
	Elf64_Xword	st_size;
} Elf64_Sym, *PElf64_Sym;

typedef struct _ELF_CONTAINER{
	UCHAR			name[2048];
	ULONG_PTR 		buffer;
	QWORD 			fileSize;
	WORD 			machine;
	union{
		PElf32_Ehdr ehdr32;
		PElf64_Ehdr ehdr64;
	} PElfN_Ehdr;
	union{
		PElf32_Shdr shdr32;
		PElf64_Shdr shdr64;
	} PElfN_Shdr;
} ELF_CONTAINER, *PELF_CONTAINER;

/**
 * Load a specific elf file into a ELF_CONTAINER used to represent it
 * @param  filename  in: thename of the file to load
 * @param  elfOfFile out: a pointer on an ELF_CONTAINER structure
 * @return           an ERROR_CODE between :
 *                      E_FILE_NOT_FOUND   if the file was not found,
 *                      E_FILE_EMPTY       if the file is empty,
 *                      E_CALLOC_ERROR     if a memory allocation failed,
 *                      E_READING_ERROR    if a reading failed,
 *                      E_NOT_ELF          if the file is not an elf file,
 *                      E_BAD_ARCHITECTURE if the file is neither an i386 file nor an amd64 file,
 *                      E_BAD_FORMAT       if the file contains errors,
 *                      E_SUCCESS          if no errors occurs,
 *                      E_FSTAT_ERROR      if fstat failed
 */
ERROR_CODE ElfInit(int fd,  CHAR* filename, PELF_CONTAINER elfOfFile);

/**
 * read all the symbols imported by an elf file, and store them into a PVECTOR
 * @param  elfOfFile    a pointer on an ELF_CONTAINER structure
 * @param  symbolVector a pointer on the PVECTOR where the symbols will be stored
 * @param  db           the PDATABASE_NODE representing the database
 * @param  db_size      the size of the database
 * @return              E_SYMBOL_TABLE_EMPTY if there is no symbols imported by the file,
 *                      E_CALLOC_ERROR       if a memory allocation failed,
 *                      E_BAD_ARCHITECTURE   if the file is neither an i386 file nor an amd64 file,
 *                      E_BAD_FORMAT         if the file contains errors,
 *                      E_SUCCESS            if no errors occurs
 */
ERROR_CODE ElfSymbolTable(PELF_CONTAINER elfOfFile, PVECTOR *symbolVector, PDATABASE_NODE db, DWORD db_size);

/**
 * free the structure
 * @param  elfOfFile a pointer on an ELF_CONTAINER
 */
VOID ElfDestroy(PELF_CONTAINER elfOfFile);

#endif /* ELF_READER_H */
