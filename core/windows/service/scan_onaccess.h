#ifndef __SCAN_ON_ACCESS_H__
#define __SCAN_ON_ACCESS_H__

#include <stdio.h>
#include <Windows.h>
#include <fltUser.h>
#include <string.h>

#include <libarmadito.h>
#include "structs.h"



#define SCAN_PORT_NAME L"\\A6oPortScanFilter"
#define USER_SCAN_THREAD_COUNT 6
#define MAX_PATH_SIZE 512
#define BUFSIZE 512

#define SCANNER_REPLY_MESSAGE_SIZE  sizeof(FILTER_REPLY_HEADER) + sizeof(SCAN_RESULT)

typedef enum a6o_file_status  SCAN_RESULT;
typedef enum a6o_file_status*  PSCAN_RESULT;

typedef struct _COMMUNICATION_PORT_CONTEXT {
	

	//
	//	Identification code of the file.
	//
	//IDENTIFICATION_MARK_FILES IdentificationFileType;

	//
	//	Type of the device where the file stands.
	//
	DEVICE_TYPE DeviceType;

	//
	//	Command given to the scan process to know what to do.
	//
	//COMMUNICATION_COMMAND Command;

	//
	//	Length of the file name.
	//
	ULONG FileNameLength;
	
	//
	//	File name buffer. This one should be enough long
	//	for almost of the normal cases. Note that, a handle
	//	to the file could be shared with user-mode rights from
	//	the kernel (case of AvScan project from M$).
	//
	CHAR FileName[MAX_PATH_SIZE];

	// Scan result Only used in the reply message.
	SCAN_RESULT scan_result;


} MESSAGE_CONTEXT, *PMESSAGE_CONTEXT; // Change to SCAN_MESSAGE_CONTEXT

typedef struct _SCANNER_MESSAGE {

    //
    //  Required structure header.
    //

    FILTER_MESSAGE_HEADER MessageHeader;

    //
    //  Private scanner-specific fields begin here.
    //

    MESSAGE_CONTEXT msg;
    
	//  Overlapped structure
    
    OVERLAPPED Ovlp;

} SCANNER_MESSAGE, *PSCANNER_MESSAGE;

typedef struct _SCANNER_REPLY_MESSAGE {

    //
    //  Required structure header.
    //

    FILTER_REPLY_HEADER ReplyHeader;

    //
    //  Private scanner-specific fields begin here.
    //

    SCAN_RESULT scanResult;
    
} SCANNER_REPLY_MESSAGE, *PSCANNER_REPLY_MESSAGE;


// Functions prototypes
HRESULT UserScanWorker(_In_  PGLOBAL_SCAN_CONTEXT Context);
HRESULT UserScanInit(_Inout_  PGLOBAL_SCAN_CONTEXT Context);
HRESULT UserScanFinalize(_In_  PGLOBAL_SCAN_CONTEXT Context);
char * ConvertDeviceNameToMsDosName(LPSTR DeviceFileName);

#endif
