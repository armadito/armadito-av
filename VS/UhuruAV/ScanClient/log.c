#include "log.h"

int SvcReportEvent(WORD eventType) {

	HANDLE hevent = NULL;

	// Register to an event log.
	hevent = RegisterEventSourceA( NULL, "UhuruAV" ); // NULL is for local computer.
	

	DeregisterEventSource(hevent);

	return -1;
}