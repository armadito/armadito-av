#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <Windows.h>
#include <fltUser.h>
#include <string.h>
#include <glib.h>

/*Libuhuru*/
#include <libuhuru-config.h>
#include <libuhuru/core.h>


#define SCAN_PORT_NAME L"\\UhuruPortScanFilter"
#define USER_SCAN_THREAD_COUNT 6
#define MAX_PATH_SIZE 512
#define BUFSIZE 512

#define SCANNER_REPLY_MESSAGE_SIZE  sizeof(FILTER_REPLY_HEADER) + sizeof(SCAN_RESULT)

typedef struct _SCANNER_THREAD_CONTEXT {

    //
    //   Threand Handle
    //

    HANDLE   Handle;

    //
    //   Threand Id
    //

    DWORD   ThreadId;

    //
    //   We need to remember scan id to know which task to abort.
    //
    
    LONGLONG  ScanId;
    
    //
    //   A flag that indicates that if this scan thread has received cancel callback from the driver
    //
    
    UINT  Aborted;

    //
    //   A critical section that synchronize the read/write of ScanId and Aborted.
    //
    
    CRITICAL_SECTION  Lock;

} SCANNER_THREAD_CONTEXT, *PSCANNER_THREAD_CONTEXT;

typedef struct _USER_SCAN_CONTEXT {

    //
    //  Scan thread contexts
    //

    PSCANNER_THREAD_CONTEXT  ScanThreadCtxes;
    
    //
    //  The abortion thread handle
    //
    
    HANDLE   AbortThreadHandle;
    
    //
    //  Finalize flag, set at UserScanFinalize(...)
    //
    
    UINT  Finalized;
    
    //
    //  Handle of connection port to the filter.
    //
    
    HANDLE   ConnectionPort;
    
    //
    //  Completion port for asynchronous message passing
    //
    
    HANDLE   Completion;

} USER_SCAN_CONTEXT, *PUSER_SCAN_CONTEXT;

/*typedef enum _SCAN_RESULT
{
	NONE = 0,		//  file not scanned yet.
	CLEAN,			//	clean file
	MALWARE,		// detected as malware.
	TIMEOUT,		// scan not finished due to timeout.
	UNDECIED,
	UNSUPPORTED,
	DBG_FLAG,
	ERR

}SCAN_RESULT_OLD, *PSCAN_RESULT_OLD;*/

typedef enum uhuru_file_status  SCAN_RESULT;
typedef enum uhuru_file_status*  PSCAN_RESULT;


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


char * ConvertDeviceNameToMsDosName(LPSTR DeviceFileName);

#endif