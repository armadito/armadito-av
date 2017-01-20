/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef ARMADITO_CORE_REPORTP_H
#define ARMADITO_CORE_REPORTP_H

#include <libarmadito/armadito.h>
#include <core/action.h>

/**
 * \struct struct a6o_report
 * \brief a structure containing the result of a file scan
 *
 * This structure is filled during a scan and passed to the scan callbacks after a
 * file has been scanned.
 *
 */

#define REPORT_PROGRESS_UNKNOWN (-1)

struct a6o_report {
	int scan_id;                          /*!< the id of the scan this report belongs to                          */
	char *path;                           /*!< the path of the scanned file                                       */
	int progress;                         /*!< the progress, can be an int (0 <= <= 100), or PROGRESS_UNKNOWN     */
	enum a6o_file_status status;          /*!< the scan status of the file (i.e. clean, malware, etc)             */
	enum a6o_action action;               /*!< the action that was executed on this file (alert, quarantine, etc) */
	char *mod_name;                       /*!< name of the module that decided the file scan status               */
	char *mod_report;                     /*!< the report of this module, usually a malware name                  */
	int malware_count;                    /*!<  number of malwares detected since scan started                    */
	int suspicious_count;                 /*!< number of suspicious files detected since scan started             */
	int scanned_count;                    /*!< number of scanned files */
};

void a6o_report_init(struct a6o_report *report, int scan_id, const char *path, int progress);

void a6o_report_destroy(struct a6o_report *report);

void a6o_report_change(struct a6o_report *report, enum a6o_file_status status, const char *mod_name, const char *mod_report);

#endif
