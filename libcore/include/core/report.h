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

struct a6o_report {
	char *path;                           /*!< the path of the scanned file                                       */
	enum a6o_file_status status;          /*!< the scan status of the file (i.e. clean, malware, etc)             */
	enum a6o_action action;               /*!< the action that was executed on this file (alert, quarantine, etc) */
	char *module_name;                    /*!< name of the module that decided the file scan status               */
	char *module_report;                  /*!< the report of this module, usually a malware name                  */
};

void a6o_report_init(struct a6o_report *report, const char *path);

void a6o_report_destroy(struct a6o_report *report);

void a6o_report_change(struct a6o_report *report, enum a6o_file_status status, const char *module_name, const char *module_report);

#endif
