#ifndef _STRUCT_H_
#define _STRUCT_H_

// Scan results struct.
// None - MALWARE - CLEAN - TIMEOUT
/*
typedef enum _SCAN_RESULT
{
	NONE = 0,		//  file not scanned yet.
	CLEAN,			//	clean file
	MALWARE,		// detected as malware.
	TIMEOUT,		// scan not finished due to timeout.
	UNDECIED,
	UNSUPPORTED,
	DBG_FLAG,
	ERROR

}SCAN_RESULT_OLD, *PSCAN_RESULT_OLD;
*/



typedef enum uhuru_file_status {
	NONE = 0,
	UHURU_UNDECIDED,         /*!< not yet decided by scan                                */
	UHURU_CLEAN,                 /*!< file is clean and does not contain a malware           */
	UHURU_UNKNOWN_FILE_TYPE,     /*!< file type is not handled by any module                 */
	UHURU_EINVAL,                /*!< an invalid value was passed to scan functions          */
	UHURU_IERROR,                /*!< an internal error occured during file scan             */
	UHURU_SUSPICIOUS,            /*!< file is suspicious: not malware but not clean also     */
	UHURU_WHITE_LISTED,          /*!< file is while listed, i.e. guaranteed clean            */
	UHURU_MALWARE,               /*!< file contains a malware                                */
	TIMEOUT,
	NOT_CONNECTED
}SCAN_RESULT, *PSCAN_RESULT;




// File context.
typedef struct _FILE_CONTEXT {

	// filename (don't need cause it's already provided by the FileObject in the FltObjects)

	// scan_result.
	SCAN_RESULT scanResult;

	// action to perform to the file (quarantine, deny access, erase, etc).

} FILE_CONTEXT, *PFILE_CONTEXT;

#endif