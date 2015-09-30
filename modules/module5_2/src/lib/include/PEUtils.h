/**
 * @file   PEUtils.h
 *
 * This file define the necessary variables, constants and types in order to read a file with MZ-PE format.
 */

#ifndef PEUTILS_H
#define PEUTILS_H

#ifndef _MSC_VER

#include "windowsTypes.h"

#define		IMAGE_NUMBEROF_DIRECTORY_ENTRIES	16
#define		IMAGE_SIZEOF_SHORT_NAME				8
#define		IMAGE_FILE_MACHINE_I386				0x014C
#define		IMAGE_FILE_MACHINE_IA64				0x0200			/* Intel Itanium */
#define		IMAGE_FILE_MACHINE_AMD64			0x8664
/* Directory Entries */
#define 	IMAGE_DIRECTORY_ENTRY_EXPORT          0   /* Export Directory */
#define 	IMAGE_DIRECTORY_ENTRY_IMPORT          1   /* Import Directory */
#define 	IMAGE_DIRECTORY_ENTRY_RESOURCE        2   /* Resource Directory */
#define 	IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   /* Exception Directory */
#define 	IMAGE_DIRECTORY_ENTRY_SECURITY        4   /* Security Directory */
#define 	IMAGE_DIRECTORY_ENTRY_BASERELOC       5   /* Base Relocation Table */
#define 	IMAGE_DIRECTORY_ENTRY_DEBUG           6   /* Debug Directory */
#define 	IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   /* Architecture Specific Data */
#define 	IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   /* RVA of GP */
#define 	IMAGE_DIRECTORY_ENTRY_TLS             9   /* TLS Directory */
#define 	IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   /* Load Configuration Directory */
#define 	IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   /* Bound Import Directory in headers */
#define 	IMAGE_DIRECTORY_ENTRY_IAT            12   /* Import Address Table */
#define 	IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   /* Delay Load Import Descriptors */
#define 	IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   /* COM Runtime descriptor */

#define		IMAGE_ORDINAL_FLAG32				0x80000000
#define 	IMAGE_ORDINAL_FLAG64 				0x8000000000000000ULL

#define 	IMAGE_FILE_SYSTEM 					0x1000			/* driver */
#define 	IMAGE_FILE_EXECUTABLE_IMAGE 		0x0002 			/* .exe */
#define 	IMAGE_FILE_DLL 						0x2000 			/* .dll */

#define 	IMAGE_DOS_SIGNATURE 				0x5A4D			/*  MZ */
#define 	IMAGE_ONLY_DOS_SIGNATURE			0x4D5A			/*  ZM */
#define 	IMAGE_NT_SIGNATURE 					0x00004550		/*  PE00 */

#define		IMAGE_NUMBEROF_DIRECTORY_ENTRIES	16

#define		USN_PAGE_SIZE						0x1000

typedef struct _IMAGE_FILE_HEADER {
	WORD 	Machine;
	WORD 	NumberOfSections;
	DWORD 	TimeDateStamp;
	DWORD 	PointerToSymbolTable;
	DWORD 	NumberOfSymbols;
	WORD 	SizeOfOptionalHeader;
	WORD 	Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
	DWORD 	VirtualAddress;
	DWORD 	Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_OPTIONAL_HEADER {
	WORD 	Magic;
	BYTE 	MajorLinkerVersion;
	BYTE 	MinorLinkerVersion;
	DWORD 	SizeOfCode;
	DWORD 	SizeOfInitializedData;
	DWORD 	SizeOfUninitializedData;
	DWORD 	AddressOfEntryPoint;
	DWORD 	BaseOfCode;
	DWORD 	BaseOfData;
	DWORD 	ImageBase;
	DWORD 	SectionAlignment;
	DWORD 	FileAlignment;
	WORD 	MajorOperatingSystemVersion;
	WORD 	MinorOperatingSystemVersion;
	WORD 	MajorImageVersion;
	WORD 	MinorImageVersion;
	WORD 	MajorSubsystemVersion;
	WORD 	MinorSubsystemVersion;
	DWORD 	Win32VersionValue;
	DWORD 	SizeOfImage;
	DWORD 	SizeOfHeaders;
	DWORD 	CheckSum;
	WORD 	Subsystem;
	WORD 	DllCharacteristics;
	DWORD 	SizeOfStackReserve;
	DWORD 	SizeOfStackCommit;
	DWORD 	SizeOfHeapReserve;
	DWORD 	SizeOfHeapCommit;
	DWORD 	LoaderFlags;
	DWORD 	NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

typedef struct _IMAGE_OPTIONAL_HEADER64 {
	WORD        Magic;
	BYTE        MajorLinkerVersion;
	BYTE        MinorLinkerVersion;
	DWORD       SizeOfCode;
	DWORD       SizeOfInitializedData;
	DWORD       SizeOfUninitializedData;
	DWORD       AddressOfEntryPoint;
	DWORD       BaseOfCode;
	ULONGLONG   ImageBase;
	DWORD       SectionAlignment;
	DWORD       FileAlignment;
	WORD        MajorOperatingSystemVersion;
	WORD        MinorOperatingSystemVersion;
	WORD        MajorImageVersion;
	WORD        MinorImageVersion;
	WORD        MajorSubsystemVersion;
	WORD        MinorSubsystemVersion;
	DWORD       Win32VersionValue;
	DWORD       SizeOfImage;
	DWORD       SizeOfHeaders;
	DWORD       CheckSum;
	WORD        Subsystem;
	WORD        DllCharacteristics;
	ULONGLONG   SizeOfStackReserve;
	ULONGLONG   SizeOfStackCommit;
	ULONGLONG   SizeOfHeapReserve;
	ULONGLONG   SizeOfHeapCommit;
	DWORD       LoaderFlags;
	DWORD       NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

typedef struct _IMAGE_NT_HEADERS {
	DWORD 					Signature;
	IMAGE_FILE_HEADER 		FileHeader;
	IMAGE_OPTIONAL_HEADER32 	OptionalHeader;
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

typedef struct _IMAGE_NT_HEADERS64 {
	DWORD 						Signature;
	IMAGE_FILE_HEADER 			FileHeader;
	IMAGE_OPTIONAL_HEADER64 	OptionalHeader;
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

typedef struct _IMAGE_DOS_HEADER {
	WORD   e_magic;
	WORD   e_cblp;
	WORD   e_cp;
	WORD   e_crlc;
	WORD   e_cparhdr;
	WORD   e_minalloc;
	WORD   e_maxalloc;
	WORD   e_ss;
	WORD   e_sp;
	WORD   e_csum;
	WORD   e_ip;
	WORD   e_cs;
	WORD   e_lfarlc;
	WORD   e_ovno;
	WORD   e_res[4];
	WORD   e_oemid;
	WORD   e_oeminfo;
	WORD   e_res2[10];
	LONG   e_lfanew;
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_IMPORT_DESCRIPTOR {
	union{
		ULONG OriginalFirstThunk;
		ULONG Characteristics;
	};
	ULONG TimeDateStamp;
	ULONG ForwarderChain;
	ULONG Name;
	ULONG FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR, *PIMAGE_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_IMPORT_BY_NAME {
	USHORT Hint;
	UCHAR Name[1];
} IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

typedef struct _IMAGE_THUNK_DATA64 {
	union {
		ULONGLONG ForwarderString;
		ULONGLONG Function;
		ULONGLONG Ordinal;
		ULONGLONG AddressOfData;
	} u1;
} IMAGE_THUNK_DATA64, *PIMAGE_THUNK_DATA64;

typedef struct _IMAGE_THUNK_DATA {
	union {
		DWORD ForwarderString;
		DWORD Function;
		DWORD Ordinal;
		DWORD AddressOfData;
	} u1;
} IMAGE_THUNK_DATA32, *PIMAGE_THUNK_DATA32;

typedef struct _IMAGE_SECTION_HEADER {
	BYTE  Name[IMAGE_SIZEOF_SHORT_NAME];
	union {
		DWORD PhysicalAddress;
		DWORD VirtualSize;
	} Misc;
	DWORD VirtualAddress;
	DWORD SizeOfRawData;
	DWORD PointerToRawData;
	DWORD PointerToRelocations;
	DWORD PointerToLinenumbers;
	WORD  NumberOfRelocations;
	WORD  NumberOfLinenumbers;
	DWORD Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef struct _IMAGE_EXPORT_DIRECTORY {
	DWORD   Characteristics;        /* 0x00 */
	DWORD   TimeDateStamp;          /* 0x04 */
	WORD    MajorVersion;           /* 0x08 */
	WORD    MinorVersion;           /* 0x0a */
	DWORD   Name;                   /* 0x0c */
	DWORD   Base;                   /* 0x10 */
	DWORD   NumberOfFunctions;      /* 0x14 */
	DWORD   NumberOfNames;          /* 0x18 */
	DWORD   AddressOfFunctions;     /* 0x1c RVA from base of image */
	DWORD   AddressOfNames;         /* 0x20 RVA from base of image */
	DWORD   AddressOfNameOrdinals;  /* 0x24 RVA from base of image */
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

typedef struct _IMAGE_DELAY_IMPORT_DESCRIPTOR {
	DWORD	Flags;
	DWORD	Name;
	DWORD	Module;
	DWORD	FirstThunk;
	DWORD	OriginalFirstThunk;
	DWORD	BoundIAT;
	DWORD	UnloadIAT;
	DWORD	TimeDateStamp;
} IMAGE_DELAY_IMPORT_DESCRIPTOR, *PIMAGE_DELAY_IMPORT_DESCRIPTOR;

#else

#include <Windows.h>

#endif

#define 	FIELD_OFFSET32(Type, Field) ((&(((Type *)(0))->Field)))
#define 	FIELD_OFFSET64(Type, Field) ((ULONG_PTR)(&(((Type *)(0))->Field)))

#define		ORDINAL_PREFIX_STRING_IMPORT		"__ordinal_import_label_"
#define		ORDINAL_PREFIX_STRING_EXPORT		"__ordinal_export_label_"

#define 	DOS_STUB 							"This program cannot be run in DOS mode.\x0D\x0D\x0A\x24"
#define 	DOS_STUB_SIZE 						44 /* taille de DOS_STUB + 1 pour le '0' */
#define 	DOS_STUB_OFFSET 					0x4E

#define 	ALT_DOS_STUB						"This program must be run under Win32\x0D\x0A\x24\x37"
#define 	ALT_DOS_STUB_SIZE 					41 /* taille de ALT_DOS_STUB + 1 pour le '0' */
#define 	ALT_DOS_STUB_OFFSET 				0x50

#define		IMAGE_DIRECTORY_ENTRY_END			15

typedef struct _PORTABLE_EXECUTABLE {
	ULONG_PTR 				BaseAddress;
	DWORD 					FileSize;
	WORD 					Machine;
	IMAGE_DOS_HEADER      	ImageDosHeader; /* 'MZ' structure */
	IMAGE_FILE_HEADER     	ImageFileHeader; /* 'Pe' structure */
	IMAGE_OPTIONAL_HEADER32 ImageOptionalHeader32; /* optional header structure */
	IMAGE_OPTIONAL_HEADER64 ImageOptionalHeader64; /* optional header structure for 64 bits*/
	PIMAGE_SECTION_HEADER   ImagesSectionHeader; /* array of section headers */
} PORTABLE_EXECUTABLE, *PPORTABLE_EXECUTABLE;

#endif /* PEUTILS_H */
