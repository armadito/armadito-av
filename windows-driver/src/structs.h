/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito windows driver.

Reproduction, distribution and derivative works are permitted under the terms of the Microsoft Public License
See file COPYING.MSPL for terms of license.

***/

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

typedef enum a6o_file_status {
	NONE = 0,
	ARMADITO_UNDECIDED,         /*!< not yet decided by scan                                */
	ARMADITO_CLEAN,                 /*!< file is clean and does not contain a malware           */
	ARMADITO_UNKNOWN_FILE_TYPE,     /*!< file type is not handled by any module                 */
	ARMADITO_EINVAL,                /*!< an invalid value was passed to scan functions          */
	ARMADITO_IERROR,                /*!< an internal error occured during file scan             */
	ARMADITO_SUSPICIOUS,            /*!< file is suspicious: not malware but not clean also     */
	ARMADITO_WHITE_LISTED,          /*!< file is while listed, i.e. guaranteed clean            */
	ARMADITO_MALWARE,               /*!< file contains a malware                                */
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