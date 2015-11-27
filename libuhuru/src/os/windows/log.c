#include "os/log.h"
#include "MessageText\uhEventProvider.h"

static void winEventHandler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data ) {

	HANDLE hevent = NULL;
	char ** msg[1];
	int numMsg = 1;
	WORD Category = 0;
	WORD EventLogType = 0;


	hevent = RegisterEventSourceA(NULL, "UhuruAV" );
	if (hevent == NULL) {
		printf("[-] Error :: SvcReportEvent!RegisterEventSourceA() failed with error :: %d\n", GetLastError( ));
		return;
	}

	//printf("[+] Debug :: Event Log Registered successfully\n" );

	msg[0] = message;
	if ( log_domain == NULL ) {
		Category = 0;
	}
	// Define Log Category
	else if (strncmp(log_domain, "LIBUHURU",strlen(log_domain)) == 0) {

		Category = LIBUHURU_CATEGORY;

	} else if(strncmp(log_domain, "MODULES",strlen(log_domain)) == 0){

		Category = MODULES_CATEGORY;

	}
	else if (strncmp(log_domain, "SERVICE",strlen(log_domain)) == 0) {
		Category = SERVICE_CATEGORY;
	}
	else {
		Category = 0;
	}


	if (ReportEventA(hevent,	// Event log handle
		EVENTLOG_WARNING_TYPE,	// Event type
		Category,						// Event category
		MSG_WARNING,						// Event identifier
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

void log_init(const char *g_log_domain)
{
  GLogLevelFlags flags;
  
  flags = G_LOG_LEVEL_ERROR|G_LOG_LEVEL_WARNING|G_LOG_LEVEL_INFO|G_LOG_LEVEL_DEBUG;

  // set log handler 
  g_log_set_handler(g_log_domain, flags, winEventHandler, NULL);
  
  return;
}