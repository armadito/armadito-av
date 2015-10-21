#ifndef _STRUCT_H_
#define _STRUCT_H_

// Scan results struct.
// None - MALWARE - CLEAN - TIMEOUT
typedef enum _SCAN_RESULT
{
	NONE = 0,		//  file not scanned yet.
	CLEAN,			//	clean file
	MALWARE,		// detected as malware.
	TIMEOUT,		// scan not finished due to timeout.
	ERROR

}SCAN_RESULT;




// File context.
typedef struct _FILE_CONTEXT {

	// filename (don't need cause it's already provided by the FileObject in the FltObjects)

	// scan_result.
	SCAN_RESULT scanResult;

	// action to perform to the file (quarantine, deny access, erase, etc).

} FILE_CONTEXT, *PFILE_CONTEXT;

#endif