/**
 * \file status.h
 *
 * \brief definition of the C enum for a file scan status
 *
 * uhuru_file_status defines the status of a file during or after scan
 */

#ifndef _LIBUHURU_COMMON_STATUS_H_
#define _LIBUHURU_COMMON_STATUS_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *uhuru_file_status_pretty_str(enum uhuru_file_status status);


enum uhuru_file_status {
  UHURU_UNDECIDED = 1,         /*!< not yet decided by scan                                */
  UHURU_CLEAN,                 /*!< file is clean and does not contain a malware           */
  UHURU_UNKNOWN_FILE_TYPE,     /*!< file type is not handled by any module                 */
  UHURU_EINVAL,                /*!< an invalid value was passed to scan functions          */
  UHURU_IERROR,                /*!< an internal error occured during file scan             */
  UHURU_SUSPICIOUS,            /*!< file is suspicious: not malware but not clean also     */
  UHURU_WHITE_LISTED,          /*!< file is while listed, i.e. guaranteed clean            */
  UHURU_MALWARE,               /*!< file contains a malware                                */
};

#ifdef __cplusplus
}
#endif

#endif
