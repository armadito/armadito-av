/**
 * \file status.h
 *
 * \brief definition of the C enum for a file scan status
 *
 * a6o_file_status defines the status of a file during or after scan
 */

#ifndef _LIBARMADITO_STATUS_H_
#define _LIBARMADITO_STATUS_H_

enum a6o_file_status {
	ARMADITO_UNDECIDED = 1,         /*!< not yet decided by scan                                */
	ARMADITO_CLEAN,                 /*!< file is clean and does not contain a malware           */
	ARMADITO_UNKNOWN_FILE_TYPE,     /*!< file type is not handled by any module                 */
	ARMADITO_EINVAL,                /*!< an invalid value was passed to scan functions          */
	ARMADITO_IERROR,                /*!< an internal error occured during file scan             */
	ARMADITO_SUSPICIOUS,            /*!< file is suspicious: not malware but not clean also     */
	ARMADITO_WHITE_LISTED,          /*!< file is while listed, i.e. guaranteed clean            */
	ARMADITO_MALWARE,               /*!< file contains a malware                                */
};

const char *a6o_file_status_str(enum a6o_file_status status);

const char *a6o_file_status_pretty_str(enum a6o_file_status status);

#endif
