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

#include "log.h"
#include <Windows.h>
#include <libarmadito.h>

int gfirst_time = 1;

void a6oLogFile(const char *fmt, ... )
{
	//static int first_time = 1;
	FILE * f = NULL;
	const char *mode = "a";
	errno_t err;
	va_list ap;

	if (gfirst_time) {
		mode = "w";
		gfirst_time = 0;
	}
	err = fopen_s(&f,"C:\\ARMADITO.TXT", mode);
	if (err != 0)
		return;

	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
	va_end(ap);
	fclose(f);

	return;
}

void winEventHandler(enum a6o_log_domain domain, enum a6o_log_level log_level, const char *message, void *user_data){

	HANDLE hevent = NULL;
	char ** msg[1];
	int numMsg = 1;
	WORD eventCategory = 0;
	//WORD eventType = 0;
	DWORD eventId = 0;
	WORD eventLogType = 0;


	hevent = RegisterEventSourceA(NULL, "Armadito-av" );
	if (hevent == NULL) {
		printf("[-] Error :: SvcReportEvent!RegisterEventSourceA() failed with error :: %d\n", GetLastError( ));
		return;
	}

	//printf("[+] Debug :: Event Log Registered successfully\n" );
	if (message != NULL)
		msg[0] = message;

	// Define Log Category
	switch(domain) {
		case A6O_LOG_LIB:
			eventCategory = LIBARMADITO_CATEGORY;
			break;
		case A6O_LOG_MODULE:
			eventCategory = MODULES_CATEGORY;
			break;
		case A6O_LOG_SERVICE:
			eventCategory = SERVICE_CATEGORY;
			break;
		default:
			eventCategory = 0;
			break;
	}

	// Define log event type.
	// TODO :: add event type :: EVENTLOG_SUCCESS - EVENTLOG_AUDIT_FAILURE - EVENTLOG_AUDIT_SUCCESS
	switch (log_level) {

		case A6O_LOG_LEVEL_ERROR:
			eventLogType = EVENTLOG_ERROR_TYPE;
			eventId = MSG_ERROR;
			break;
		case A6O_LOG_LEVEL_WARNING:
			eventLogType = EVENTLOG_WARNING_TYPE;
			eventId = MSG_WARNING;
			break;
		case A6O_LOG_LEVEL_INFO:
			eventLogType = EVENTLOG_INFORMATION_TYPE;
			eventId = MSG_INFO;
			break;
		case A6O_LOG_LEVEL_DEBUG:
			eventLogType = EVENTLOG_INFORMATION_TYPE;
			eventId = MSG_INFO;
			break;
		case A6O_LOG_LEVEL_NONE:
			eventLogType = EVENTLOG_INFORMATION_TYPE;
			eventId = MSG_INFO;
			break;
		default:
			eventLogType = EVENTLOG_INFORMATION_TYPE;
			eventId = MSG_INFO;
			break;
	}

	if (ReportEventA(hevent,	// Event log handle
		eventLogType,			// Event type
		eventCategory,			// Event category
		eventId,				// Event identifier
		NULL,					// Security identifier
		numMsg,					// Size of the string array
		0,						// Binary data
		msg,					// Array of strings
		NULL					// Binary data
		) == FALSE) {
		printf("[-] Error :: SvcReportEvent!ReportEventA() failed with error :: %d\n",GetLastError());
	}
	else {
		//printf("[+] Debug :: Report Event executed successfully\n" );
	}

	DeregisterEventSource(hevent);

	return;
}
