#include "service.h"
#include "log.h"
#include "scan_on_access.h"
#include "uh_crypt.h"
#include "update.h"
#include "register.h"
#include "uh_info.h"

// Msdn documentation: 
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms685141%28v=vs.85%29.aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms687416%28v=vs.85%29.aspx

SERVICE_STATUS gSvcStatus;
SERVICE_STATUS_HANDLE gSvcStatusHandle;
HANDLE ghSvcStopEvent = NULL;

struct uhuru * uhuru = NULL;
USER_SCAN_CONTEXT userScanCtx = {0};
ONDEMAND_SCAN_CONTEXT onDemandCtx = {0};

/*------------------------------------------------
	Service Load and unload procedures functions
--------------------------------------------------*/


int ServiceLoadProcedure( ) {

	int ret = 0;
	uhuru_error * uh_error = NULL;
	HRESULT hres = S_OK;

	__try {

		// Init uhuru structure
		uhuru = uhuru_open(&uh_error);
		if (uhuru == NULL) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " uhuru_open() struct initialization failed!\n");
			ret = -1;
			__leave;
		}

		//  Initialize scan listening threads. and Connect to the driver communication port. (Only if real time is enabled)
		if (REAL_TIME_ENABLED) {
			hres = UserScanInit(&userScanCtx);
			if (FAILED(hres)) {
				//hres = UserScanFinalize(&userScanCtx);
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Scan Thread initialization failed!\n");
				ret = -2;
				__leave;
			}

			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_INFO, " Service connected to the driver successfully!\n");
		}
		

		// Create Named Pipe for IHM
		// Notes : If you intend to use a named pipe locally only, deny access to NT AUTHORITY\NETWORK or switch to local RPC.
		if (Start_IHM_Connection(uhuru, &onDemandCtx) < 0) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," Start IHM connection failed :: %d\n",ret);
			ret = -3;
			__leave;

		}	
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_INFO, " Service connected to the GUI successfully!\n");


	}
	__finally {

		// if failed
		if (ret < 0) {			

			// Finalyze listening threads and Close communication port.
			if (ret < -1 && REAL_TIME_ENABLED)
				hres = UserScanFinalize(&userScanCtx);

			// Close Named Pipe
			if (ret < -2) {
				ret = Close_IHM_Connection(&onDemandCtx);
			}

			//close uhuru structure
			if (uhuru != NULL) {
				uhuru_close(uhuru,&uh_error);
				uhuru = NULL;
			}
			
		}

	}

	return ret;
}

int ServiceUnloadProcedure( ) {

	HRESULT hres = S_OK;
	uhuru_error * uh_error = NULL;
	int ret = 0;

	// Finish all scan threads and close communication port with driver.	
	if (REAL_TIME_ENABLED) {
		hres = UserScanFinalize(&userScanCtx);
		if (FAILED(hres)) {
			 ret = -1;
		}
	}
	
	// Finish all thread and Close the pipe.
	ret = Close_IHM_Connection(&onDemandCtx);

	// Close Uhuru structure
	if (uhuru != NULL) {
		uhuru_close(uhuru, &uh_error);
		uhuru = NULL;
	}		

	return ret;
}


// RegistryInitialization()
int RegistryKeysInitialization( ) {

	//char * subkey = "SYSTEM\\CurrentControlSet\\services\\eventlog\\Application\\Tatou";
	
	LONG res = 0;
	INT ret = -1;
	HKEY hRootkey = NULL;
	HKEY hkey = NULL;
	DWORD dwValue = 3;
	DWORD dwTypes = 7;
	LPSTR dllpath = APP_DLL_PATH;
	LPSTR dumpfolder = DUMP_FOLDER;
	DWORD dwDumpType = DUMP_TYPE;
	DWORD size = 0;

	__try {

		// Open the main service key
		res = RegOpenKeyA(HKEY_LOCAL_MACHINE, ROOT_KEY_PATH, &hRootkey);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegOpenKeyA failed with error :: %d :: %d\n", GetLastError( ), res);
			__leave;
		}
		//printf("[+] Debug :: initRegistrykeys :: root key %s opened successfully\n", ROOT_KEY_PATH);

		res = RegCreateKeyA(hRootkey,APPS_KEY_NAME,&hkey);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegCreateKeyA failed with error :: %d :: %d\n", GetLastError( ), res);
			// TODO :: if the key is already created.
			__leave;
		}
		//printf("[+] Debug :: initRegistrykeys :: key %s\\%s created successfully\n",ROOT_KEY_PATH,APPS_KEY_NAME);

		// Set keys values (CategoryCount - CategoryMessageFile - EventMessageFile - )
		res = RegSetKeyValueA(hkey,NULL,"CategoryCount",REG_DWORD,&dwValue,sizeof(DWORD));
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegSetKeyValueA failed with error :: %d :: %d\n", GetLastError(),res );
			__leave;
		}

		size = strnlen(dllpath, MAX_PATH) +1;
		
		res = RegSetKeyValueA(hkey,NULL,"CategoryMessageFile",REG_EXPAND_SZ,dllpath,size);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegSetKeyValueA failed with error :: %d :: %d\n", GetLastError(),res );
			__leave;
		}

		res = RegSetKeyValueA(hkey,NULL,"EventMessageFile",REG_EXPAND_SZ,dllpath,size);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegSetKeyValueA failed with error :: %d :: %d\n", GetLastError(),res);
			__leave;
		}

		res = RegSetKeyValueA(hkey,NULL,"ParameterMessageFile",REG_EXPAND_SZ,dllpath,size);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegSetKeyValueA failed with error :: %d :: %d\n", GetLastError(),res );
			__leave;
		}

		res = RegSetKeyValueA(hkey,NULL,"TypesSupported",REG_DWORD,&dwTypes,sizeof(DWORD));
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegSetKeyValueA failed with error :: %d :: %d\n", GetLastError(),res );
			__leave;
		}

		
		//ret = 0;

	}
	__finally {

		if (hRootkey != NULL) {
			RegCloseKey(hRootkey);
			hRootkey = NULL;
		}

		if (hkey != NULL) {
			RegCloseKey(hkey);
			hkey = NULL;
		}

	}

	// Crash report configuration registry key.
	__try {

		ret = -1;

		// Open the main service key
		res = RegOpenKeyA(HKEY_LOCAL_MACHINE, ROOT_CRASH_KEY_PATH_LOCAL_DUMPS, &hRootkey);
		if (res != ERROR_SUCCESS) {
			
			if (res == ERROR_FILE_NOT_FOUND) { // if the LocalDumps key is not created, then create it.

				// 
				res = RegOpenKeyA(HKEY_LOCAL_MACHINE, ROOT_CRASH_KEY_PATH, &hRootkey);
				if (res != ERROR_SUCCESS) {
					printf("[-] Error :: RegOpenKeyA failed with error :: %d :: %d\n", GetLastError( ), res);
					__leave;
				}

				res = RegCreateKeyA(hRootkey,"LocalDumps",&hkey);
				if (res != ERROR_SUCCESS) {
					printf("[-] Error :: RegCreateKeyA failed with error :: %d :: %d\n", GetLastError( ), res);					
					__leave;
				}

				if (hRootkey != NULL) {
					RegCloseKey(hRootkey);
					hRootkey = NULL;
				}

				if (hkey != NULL) {
					RegCloseKey(hkey);
					hkey = NULL;
				}

				res = RegOpenKeyA(HKEY_LOCAL_MACHINE, ROOT_CRASH_KEY_PATH_LOCAL_DUMPS, &hRootkey);
				if (res != ERROR_SUCCESS) {
					printf("[-] Error :: RegistryKeysInitialization :: RegOpenKeyA() failed with error :: %d :: %d\n", GetLastError( ), res);
					__leave;
				}

			}
			else {
				printf("[-] Error :: RegOpenKeyA failed with error :: %d :: %d\n", GetLastError( ), res);
				__leave;
			}
			
		}		


		res = RegCreateKeyA(hRootkey,SVC_KEY_NAME,&hkey);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegCreateKeyA failed with error :: %d :: %d\n", GetLastError( ), res);			
			__leave;
		}

		// Set dump folder
		size = strnlen(dumpfolder, MAX_PATH) +1;
		res = RegSetKeyValueA(hkey,NULL,"DumpFolder",REG_EXPAND_SZ,DUMP_FOLDER,size);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegSetKeyValueA failed with error :: %d :: %d\n", GetLastError(),res );
			__leave;
		}

		res = RegSetKeyValueA(hkey,NULL,"DumpType",REG_DWORD,&dwDumpType,sizeof(DWORD));
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegSetKeyValueA failed with error :: %d :: %d\n", GetLastError(),res );
			__leave;
		}

		printf("[+] Debug :: RegistryKeysInitialization :: UhuruAV event log keys values created successfully\n" );
		ret = 0;

	}
	__finally {

		if (hRootkey != NULL) {
			RegCloseKey(hRootkey);
			hRootkey = NULL;
		}

		if (hkey != NULL) {
			RegCloseKey(hkey);
			hkey = NULL;
		}

	}

	// Driver Registry keys for crash dumps (verification only)
	/*
	__try {

		ret = -1;

		// Open the main service key
		res = RegOpenKeyA(HKEY_LOCAL_MACHINE, ROOT_DRIVER_CRASH_KEY_PATH, &hRootkey);
		if (res != ERROR_SUCCESS) {
			__leave;
		}

		// TODO...

		ret = 0;

	}
	__finally {

		if (hRootkey != NULL) {
			RegCloseKey(hRootkey);
			hRootkey = NULL;
		}

		if (hkey != NULL) {
			RegCloseKey(hkey);
			hkey = NULL;
		}

	}
	*/


	return ret;
}

int DeleteRegistryKeys( ) {

	LONG res = 0;
	HKEY hRootkey = NULL;	
	DWORD dwValue = 3;
	DWORD dwTypes = 7;
	LPSTR dllpath = APP_DLL_PATH;
	DWORD size = 0;
	int ret = 0;

	__try {

		// Open the main service key
		res = RegOpenKeyA(HKEY_LOCAL_MACHINE, ROOT_KEY_PATH, &hRootkey);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegOpenKeyA failed with error :: %d :: %d\n", GetLastError( ), res);
			ret = -1;
			__leave;
		}
		printf("[+] Debug :: DeleteRegistrykeys :: root key %s opened successfully\n", ROOT_KEY_PATH);

		// Delete the existing key
		res = RegDeleteKeyA(hRootkey, APPS_KEY_NAME);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegDeleteKeyA failed with error :: %d :: %d\n", GetLastError(),res );
			ret = -1;
			__leave;
		}
		
		printf("[+] Debug :: DeleteRegistrykeys :: key %s\\%s deleted successfully\n", ROOT_KEY_PATH,APPS_KEY_NAME);


	}
	__finally {

		if (hRootkey != NULL) {
			RegCloseKey(hRootkey);
			hRootkey = NULL;
		}

	}

	__try {

		// Open the main service key
		res = RegOpenKeyA(HKEY_LOCAL_MACHINE, ROOT_CRASH_KEY_PATH_LOCAL_DUMPS, &hRootkey);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegOpenKeyA failed with error :: %d :: %d\n", GetLastError( ), res);
			ret = -1;
			__leave;
		}
		printf("[+] Debug :: DeleteRegistrykeys :: root key %s opened successfully\n", ROOT_CRASH_KEY_PATH_LOCAL_DUMPS);

		// Delete the existing key
		res = RegDeleteKeyA(hRootkey, SVC_KEY_NAME);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegDeleteKeyA failed with error :: %d :: %d\n", GetLastError(),res );
			ret = -1;
			__leave;
		}
		
		printf("[+] Debug :: DeleteRegistrykeys :: key %s\\%s deleted successfully\n", ROOT_CRASH_KEY_PATH_LOCAL_DUMPS,SVC_KEY_NAME);


	}
	__finally {

		if (hRootkey != NULL) {
			RegCloseKey(hRootkey);
			hRootkey = NULL;
		}

	}


	return 0;
}


/*
 int ServiceInstall()
 https://msdn.microsoft.com/en-us/library/windows/desktop/ms683500%28v=vs.85%29.aspx

*/
int ServiceInstall( ) {

	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	char binaryPath[MAX_PATH] = {'\0'};
	int ret = -1;


	__try {

		// Get the path of the current binary
		if (!GetModuleFileName(NULL, binaryPath, MAX_PATH)) {
			printf("[-] Error :: ServiceInstall!GetModuleFileName() failed :: exit_code %d\n",GetLastError());
			__leave;
		}

		// Get a handle to the SCM database.
		schSCManager = OpenSCManager(NULL,						// Local computer.
									 NULL,						// ServicesActive database.
									 SC_MANAGER_ALL_ACCESS) ;	// full access rights.

		if (schSCManager == NULL) {
			printf("[-] Error :: ServiceInstall!OpenSCManager() failed :: exit_code = %d\n",GetLastError());
			__leave;
		}

		// Create the service.
		schService = CreateService( 
			schSCManager,				// SCM database 
			SVCNAME,					// name of service 
			SVCDISPLAY,					// service name to display 
			SERVICE_ALL_ACCESS,			// desired access 
			SERVICE_WIN32_OWN_PROCESS,	// service type 
			SERVICE_DEMAND_START,		// start type
			SERVICE_ERROR_NORMAL,		// error control type 
			binaryPath,					// path to service's binary 
			NULL,						// no load ordering group 
			NULL,						// no tag identifier 
			NULL,						// no dependencies 
			NULL,						// LocalSystem account 
			NULL);						// no password 

		if (schService == NULL) {
			printf("[-] Error :: ServiceInstall!CreateService() failed :: exit_code = %d\n",GetLastError());			
			__leave;
		}

		if (RegistryKeysInitialization( ) < 0) {
			printf("[-] Warning :: Service Registry key creation failed!\n");		
			__leave;
		}
		

		ret = 0;
		printf("[+] Debug :: Service installed successfully!\n");
		uhLog("[+] Debug :: Service installed successfully!\n");		

	}
	__finally {

		if (schSCManager != NULL) {
			CloseServiceHandle(schSCManager);
		}

		if (schService != NULL) {			
			CloseServiceHandle(schService);
		}

	}

	return ret;
}


/*
	ServiceRemove()
	https://msdn.microsoft.com/en-us/library/windows/desktop/ms682571%28v=vs.85%29.aspx
*/
int ServiceRemove( ) {

	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwBytesNeeded;

	// Get a handle to the SCM database.
	schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (schSCManager == NULL){
        printf("[-] Error :: ServiceRemove!OpenSCManager() failed (%d)\n", GetLastError());
        return -1;
    }

	// Get a handle to the service.
	schService = OpenService(schSCManager, SVCNAME, DELETE|SERVICE_QUERY_STATUS);
	if (schService == NULL) {
		printf("[-] Error :: ServiceRemove!OpenService() failed :: exit_code = %d\n",GetLastError());
		CloseServiceHandle(schSCManager);
		return -1;
	}
	
	// Check the status in case the service is started. If it's the case, then stop it first.
	if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
		printf("[-] Error :: ServiceRemove!QueryServiceStatusEx() failed :: exit_code = %d\n",GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return -1;
	}

	if (ssStatus.dwCurrentState == SERVICE_RUNNING || ssStatus.dwCurrentState == SERVICE_START_PENDING ) {
		printf("[i] Debug :: ServiceRemove :: Stopping the service...\n");
		ServiceStop( );
	}

	// Delete the service
	if (!DeleteService(schService)) {
		printf("[-] Error :: ServiceRemove!DeleteService() failed :: exit_code = %d\n",GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return -1;
	}
	
	// Delete Registry keys
	DeleteRegistryKeys( );

	printf("[+] Debug :: Service deleted sucessfully !\n");


	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return 0;
}


/*
	ReportSvcStatus()
	https://msdn.microsoft.com/en-us/library/windows/desktop/ms687414%28v=vs.85%29.aspx
*/
VOID ReportSvcStatus(DWORD dwCurrentState,
					 DWORD dwWin32ExitCode,
					 DWORD dwWaitHint) {

	static DWORD dwCheckPoint = 1;

	// Fill the service status structure

	gSvcStatus.dwCurrentState = dwCurrentState;
	gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
	gSvcStatus.dwWaitHint = dwWaitHint;

	if (gSvcStatus.dwCurrentState == SERVICE_START_PENDING) {
		gSvcStatus.dwControlsAccepted = 0;
	}
	else
		gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_PAUSE_CONTINUE;

	if (dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED)
		gSvcStatus.dwCheckPoint = 0;
	else
		gSvcStatus.dwCheckPoint = dwCheckPoint++;

	// Report the status of the service to the SCM.
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);

	return;

}





/*
	ServiceCtrlHandler()
	https://msdn.microsoft.com/en-us/library/windows/desktop/ms687413%28v=vs.85%29.aspx
	https://msdn.microsoft.com/en-us/library/windows/desktop/ms685149%28v=vs.85%29.aspx
*/
void WINAPI ServiceCtrlHandler( DWORD dwCtrl ) {

	uhuru_error * uh_error = NULL;
	HRESULT hres = S_OK;
	int ret = 0;

	switch (dwCtrl) {

		case SERVICE_CONTROL_PAUSE:

			ReportSvcStatus(SERVICE_PAUSE_PENDING, NO_ERROR, 0);
			
			// Unload service.
			ret = ServiceUnloadProcedure( );
			if (ret != 0) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Service unloaded with errors during pause.\n");
				uhLog("[-] Error :: Service unloaded with errors\n");
			}

			ReportSvcStatus(SERVICE_PAUSED, NO_ERROR, 0);
			uhLog("[i] DEBUG :: SERVICE PAUSE !!!\n");
			break;

		case SERVICE_CONTROL_CONTINUE:

			ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 0);
			ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
			ret = ServiceLoadProcedure( );
			if (ret < 0) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Service Initialization failed during continue \n");
				uhLog("[+] Error :: Service Initialization failed\n");
				// Stop the service on error.
				ServiceStop( );
			}
			uhLog("[i] DEBUG :: SERVICE CONTINUE !!!\n");
			break;

		case SERVICE_CONTROL_STOP:

			ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

			ret = ServiceUnloadProcedure( );
			if (ret != 0) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Service unloaded with errors\n");
				uhLog("[-] Error :: Service unloaded with errors\n");
			}			

			 // Signal the service to stop.
			 SetEvent(ghSvcStopEvent);
			 ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);

			return;
		case SERVICE_CONTROL_INTERROGATE:
			break;
		//TODO ::  add case SERVICE_CONTROL_PRESHUTDOWN
		default:
			break;
	}

	return;
}


/*
 LaunchServiceAction()
*/
void PerformServiceAction( ) {
	
	uhuru_error * uh_error = NULL;
	HRESULT hres = S_OK;
	int ret = 0;

	// set log handler (windows log event) // move this statement to a better place.
	uhuru_log_set_handler(UHURU_LOG_LEVEL_NONE, winEventHandler,NULL);

	ret = ServiceLoadProcedure( );
	if (ret < 0) {
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Service Initialization failed \n");
		uhLog("[+] Error :: Service Initialization failed\n");
		// Stop the service on error.
		ServiceStop( );

	}
	else {
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_INFO, " Service Initializaed successfully!\n");
		uhLog("[+] Debug :: uhuru struct initialized successfully\n");
	}
	
	return;
}

/*
	ServiceInit()
	https://msdn.microsoft.com/en-us/library/windows/desktop/ms687414%28v=vs.85%29.aspx
*/
void ServiceInit( ) {

	// TODO :: Declare and set any required Variables.
	// Be sure to periodically call ReportSvcStatus() with SERVICE_START_PENDING.
	// If Initialization failed, then call ReportSvcStatus() with SERVICE_STOPPED.
	// ...


	// Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.
	ghSvcStopEvent = CreateEvent(
                         NULL,    // default security attributes
                         TRUE,    // manual reset event
                         FALSE,   // not signaled
                         NULL);   // no name

	if (ghSvcStopEvent == NULL){
        ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
        return;
    }

	// Report running status when initialization is complete.
    ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );


	return;

}


/*
	ServiceMain function.
	https://msdn.microsoft.com/en-us/library/windows/desktop/ms687414%28v=vs.85%29.aspx
*/
void WINAPI ServiceMain(int argc, char ** argv) {


	// first call the RegisterServiceCtrlHandler. Register the SvcHandler function as the service's handler function.
	gSvcStatusHandle = RegisterServiceCtrlHandler( SVCNAME,(LPHANDLER_FUNCTION)&ServiceCtrlHandler);
	if (gSvcStatusHandle == NULL) {		
		//SvcReportEvent(TEXT("RegisterServiceCtrl"));
		// Write in a log file
		// Stop the service on error.
		ServiceStop( );
		return;
	}

	//gSvcStatus.dwControlsAccepted = p
	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	gSvcStatus.dwServiceSpecificExitCode = 0;


	// Call the ReportSvcStatus function to indicate that its initial status is SERVICE_START_PENDING.
	ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	// Calls the SvcInit function to perform the service-specific initialization and begin the work to be performed by the service.
	ServiceInit( );

	PerformServiceAction( );



	return;
}


BOOLEAN ServiceLaunchAction( ) {

	SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { SVCNAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain }, 
        { NULL, NULL } 
    }; 


	return TRUE;
}


/*
	ServiceStart()s
	https://msdn.microsoft.com/en-us/library/windows/desktop/ms686315%28v=vs.85%29.aspx
*/
void ServiceLaunch( ) {

	SERVICE_STATUS_PROCESS ssStatus;
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	DWORD dwBytesNeeded;
	DWORD dwOldCheckPoint;
	DWORD dwStartTickCount;
	DWORD dwWaitTime;


	// Get a handle to the SCM
	schSCManager = OpenSCManager(NULL, NULL,SC_MANAGER_ALL_ACCESS) ;
	if (schSCManager == NULL) {
		printf("[-] Error :: ServiceLaunch!OpenSCManager() failed :: exit_code = %d\n",GetLastError());
		return;
	}

	// Get a handle to the srevice.
	schService = OpenService(schSCManager, SVCNAME, SERVICE_ALL_ACCESS);
	if (schService == NULL) {
		printf("[-] Error :: ServiceLaunch!OpenService() failed :: exit_code = %d\n",GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Check the status in case the service is already started.
	if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
		printf("[-] Error :: ServiceLaunch!QueryServiceStatusEx() failed :: exit_code = %d\n",GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING ) {

		printf("[i] Warning :: ServiceLaunch :: Cannot start the service because it is already runnnig\n");
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// TODO :: Handle the case where the service is pending to stop.


	// Attempt to start the service.
	if (!StartService(schService,0, NULL)) {
		printf("[-] Error :: ServiceLaunch!StartService() failed :: exit_code = %d\n",GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}
	else 
		printf("[-] Debug :: ServiceLaunch!StartService() :: Service start pending...\n");


	// Check the status until the service is no longer start pending.

	if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
		printf("[-] Error :: ServiceLaunch!QueryServiceStatusEx() failed :: exit_code = %d\n",GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	dwStartTickCount = GetTickCount( );
	dwOldCheckPoint = ssStatus.dwCheckPoint;


	while (ssStatus.dwCurrentState == SERVICE_START_PENDING) {


		// Wait a time.
		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000) {
			dwWaitTime = 1000;
		}
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
			printf("[-] Error :: ServiceLaunch!QueryServiceStatusEx() failed :: exit_code = %d\n",GetLastError());
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			break;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
			// Continue to wait and check
			dwStartTickCount = GetTickCount( );
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else {
			if (GetTickCount( ) - dwStartTickCount > ssStatus.dwWaitHint) {

				printf("[i] Warning :: ServiceLaunch :: No progress made within the wait hint\n" );
				// No progress made within the wait hint
				break;
			}
		}
	}

	// Determine whether the service is running.
	if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
        printf("[+] Info :: ServiceLaunch :: Service started successfully !\n" );	

	} else {
		printf("[i] Error :: ServiceLaunch :: Service not started\n" );
		printf("[i] Error :: ServiceLaunch :: Current State: %d\n", ssStatus.dwCurrentState  );
		printf("[i] Error :: ServiceLaunch :: Exit code: %d\n", ssStatus.dwWin32ExitCode );
		printf("[i] Error :: ServiceLaunch :: Check code: %d\n", ssStatus.dwCheckPoint );
		printf("[i] Error :: ServiceLaunch :: Wait Hint: %d\n", ssStatus.dwWaitHint );
    } 

	CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);

	return;
}


/*
	ServiceStop()
	https://msdn.microsoft.com/en-us/library/windows/desktop/ms686335%28v=vs.85%29.aspx
*/
void ServiceStop( ) {

	SERVICE_STATUS_PROCESS ssStatus;
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	DWORD dwBytesNeeded;
	DWORD dwStartTime = GetTickCount( );
	DWORD dwTimeout = 10000;
	DWORD exitCode = 0;
	int ret = 0;


	// Get a handle to the SCM database. 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (schSCManager == NULL) {
		printf("[-] Error :: ServiceStop!OpenSCManager() failed :: exit_code = %d\n",GetLastError());
		return ;
	}

	// Get a handle to the Service.
	schService = OpenService(schSCManager, SVCNAME, SERVICE_STOP|SERVICE_QUERY_STATUS|SERVICE_ENUMERATE_DEPENDENTS);
	if (schService == NULL) {
		printf("[-] Error :: OpenService() function failed :: exit_code = %d\n",GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Make sure the service is not already stopped.
	if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
		printf("[-] Error :: ServiceStop!QueryServiceStatusEx() failed :: exit_code = %d\n",GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	if (ssStatus.dwCurrentState == SERVICE_STOPPED) {
		printf("[i] Warning :: ServiceStop :: The service is already stopped !\n");
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// If a stop is pending, wait for it.
	// TODO : Set service stop timeout.
	while ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
		
		printf("[i] Debug :: ServiceStop :: Service Stop pending...\n");

		if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
			printf("[-] Error :: ServiceStop!QueryServiceStatusEx() failed :: exit_code = %d\n",GetLastError());
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return;
		}

		if (ssStatus.dwCurrentState == SERVICE_STOPPED) {
			printf("[+] Debug :: ServiceStop :: Service stopped successfully !\n");			
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return;
		}

	}

	// If the service is running, dependencies must be stopped first. (In our case, there is no dependencies).	
	// TODO: Unload databases, + modules + others dependencies...
	ret = ServiceUnloadProcedure( );
	if (ret != 0) {
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Service unloaded with errors\n");
		uhLog("[-] Error :: Service unloaded with errors\n");
	}	

	// Send a stop code to the service.
	if (!ControlService(schService,SERVICE_CONTROL_STOP,(LPSERVICE_STATUS)&ssStatus)) {
		printf("[-] Error :: ServiceStop!ControlService() failed :: exit_code = %d\n",GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// Wait for the service to stop.
	while ( ssStatus.dwCurrentState != SERVICE_STOPPED) {

		Sleep(ssStatus.dwWaitHint);

		if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
			printf("[-] Error :: ServiceStop!QueryServiceStatusEx() failed :: exit_code = %d\n",GetLastError());
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return;
		}

		if (ssStatus.dwCurrentState == SERVICE_STOPPED) {			
			break;
		}

		if (GetTickCount( ) - dwStartTime > dwTimeout) {
			printf("[-] Error :: ServiceStop :: Service Stop failed :: Stop Wait timeout !\n");
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return;
		}

	}
	printf("[+] Debug :: Service stopped successfully\n");


	return;
}

/*Pause the service*/
int ServicePause( ) {

	int ret = 0;
	SERVICE_STATUS_PROCESS ssStatus;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	DWORD dwBytesNeeded;
	DWORD dwStartTime = GetTickCount( );
	DWORD dwTimeout = 10000;

	__try {

		// Get a handle to the SCM database. 
		schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (schSCManager == NULL) {
			printf("[-] Error :: ServicePause :: OpenSCManager() failed :: GLE = %d\n",GetLastError());
			ret = -1;
			__leave;			
		}

		// Get a handle to the Service.
		schService = OpenService(schSCManager, SVCNAME, SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS|SERVICE_ENUMERATE_DEPENDENTS);
		if (schService == NULL) {
			printf("[-] Error :: ServicePause :: Open Service failed :: GLE = %d\n",GetLastError());
			ret = -2;
			__leave;			
		}

		// Make sure the service is running.
		if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
			printf("[-] Error :: ServicePause :: Query Service Status failed :: GLE = %d\n",GetLastError());
			ret = -3;
			__leave;
		}

		if (ssStatus.dwCurrentState != SERVICE_RUNNING) {
			printf("[-] Error :: ServicePause :: The service is not running!\n");
			ret = -4;
			__leave;
		}

		// Send a pause code to the service.
		if (!ControlService(schService,SERVICE_CONTROL_PAUSE,(LPSERVICE_STATUS)&ssStatus)) {
			printf("[-] Error :: ServicePause :: Control Service failed :: GLE = %d\n",GetLastError());
			ret = -5;
			__leave;
		}

		// Wait for the service to stop.
		while (ssStatus.dwCurrentState != SERVICE_STOPPED) {

			Sleep(ssStatus.dwWaitHint);

			if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
				printf("[-] Error :: ServicePause :: Query Service Status failed :: GLE = %d\n",GetLastError());
				ret = -6;
				__leave;
			}

			if (ssStatus.dwCurrentState == SERVICE_PAUSED) {			
				break;
			}

			if (GetTickCount( ) - dwStartTime > dwTimeout) {
				printf("[-] Error :: ServiceStop :: Service Pause failed :: Pause Wait timeout !\n");
				ret = -7;
				__leave;
			}

		}

		printf("[+] Debug :: ServicePause :: Service paused successfully!\n");


	}
	__finally {

		if (schSCManager != NULL) {
			CloseServiceHandle(schSCManager);
			schService = NULL;
		}

		if (schService != NULL) {
			CloseServiceHandle(schService);
			schService = NULL;
		}

	}

	

	return ret;
}

/*Resume the service*/
int ServiceContinue( ) {

	int ret = 0;
	SERVICE_STATUS_PROCESS ssStatus;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	DWORD dwBytesNeeded;
	DWORD dwStartTime = GetTickCount( );
	DWORD dwTimeout = 10000;

	__try {

		// Get a handle to the SCM database. 
		schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (schSCManager == NULL) {
			printf("[-] Error :: ServiceContinue :: OpenSCManager() failed :: GLE = %d\n",GetLastError());
			ret = -1;
			__leave;			
		}

		// Get a handle to the Service.
		schService = OpenService(schSCManager, SVCNAME, SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS|SERVICE_ENUMERATE_DEPENDENTS);
		if (schService == NULL) {
			printf("[-] Error :: ServiceContinue :: Open Service failed :: GLE = %d\n",GetLastError());
			ret = -2;
			__leave;			
		}

		// Make sure the service is running.
		if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
			printf("[-] Error :: ServiceContinue :: Query Service Status failed :: GLE = %d\n",GetLastError());
			ret = -3;
			__leave;
		}

		if (ssStatus.dwCurrentState != SERVICE_PAUSED) {
			printf("[-] Error :: ServiceContinue :: The service is not paused!\n");
			ret = -4;
			__leave;
		}

		// Send a pause code to the service.
		if (!ControlService(schService,SERVICE_CONTROL_CONTINUE,(LPSERVICE_STATUS)&ssStatus)) {
			printf("[-] Error :: ServiceContinue :: Control Service failed :: GLE = %d\n",GetLastError());
			ret = -5;
			__leave;
		}

		// Wait for the service to stop.
		while (ssStatus.dwCurrentState != SERVICE_RUNNING) {

			Sleep(ssStatus.dwWaitHint);

			if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
				printf("[-] Error :: ServiceContinue :: Query Service Status failed :: GLE = %d\n",GetLastError());
				ret = -6;
				__leave;
			}

			if (ssStatus.dwCurrentState == SERVICE_RUNNING) {			
				break;
			}

			if (GetTickCount( ) - dwStartTime > dwTimeout) {
				printf("[-] Error :: ServiceContinue :: Service Continue failed :: Continue Wait timeout !\n");
				ret = -7;
				__leave;
			}

		}

		printf("[+] Debug :: ServiceContinue :: Service resumed successfully!\n");


	}
	__finally {

		if (schSCManager != NULL) {
			CloseServiceHandle(schSCManager);
			schService = NULL;
		}

		if (schService != NULL) {
			CloseServiceHandle(schService);
			schService = NULL;
		}

	}

	

	return ret;
}

int LaunchCmdLineServiceGUI( ) {

	int ret = 0;
	unsigned char c;
	uhuru_error * uh_error = NULL;
	HRESULT hres = S_OK;
	
	// Init uhuru structure
	uhuru = uhuru_open(&uh_error);
	if (uhuru == NULL) {
		printf("[-] Error :: uhuru_open() struct initialization failed!\n");
		return -1;
	}

	ret = Start_IHM_Connection(uhuru,&onDemandCtx);

	while(1) {
		 printf("press 'q' to quit: \n");
        c = (unsigned char) getchar();
        if (c == 'q') {        
            break;
		}
		else if (c == 'p') {

			// pause.
			ret = ServiceUnloadProcedure( );
			if (ret != 0) {
				//uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Service unloaded with errors during pause.\n");
				//uhLog("[-] Error :: Service unloaded with errors\n");
				printf("[-] Error :: Service Unload Procedure failed! :: %d\n", ret);
				break;
			}
			

		}
		else if (c == 'c') {

			// continue.			
			ret = ServiceLoadProcedure( );
			if (ret < 0) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Service Initialization failed during continue \n");
				uhLog("[+] Error :: Service Initialization failed\n");
				printf("[-] Error :: Service Load Procedure failed! :: %d\n", ret);
			}

		}

	}

	ret = Close_IHM_Connection(&onDemandCtx);

	if (uhuru != NULL) {
		uh_error = NULL;
		uhuru_close(uhuru,&uh_error);
		uhuru = NULL;
	}

	return ret;

}

int LaunchCmdLineService( ) {

	int ret = 0;
	unsigned char c;
	uhuru_error * uh_error = NULL;
	HRESULT hres = S_OK;


	__try {
		
		if (ServiceLoadProcedure( ) < 0) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Service Initialization failed:\n");
			//uhLog("[+] Error :: Service Initialization failed\n");
			printf("[-] Error :: Service Load Procedure failed! :: %d\n", ret);
			__leave;
		}

		while(1) {

			printf("press 'q' to quit: \n");
			c = (unsigned char) getchar();


			if (c == 'q') {
				__leave;
			}
			else if (c == 'p') {

				// pause.				
				if (ServiceUnloadProcedure( ) != 0) {
					//uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Service unloaded with errors during pause.\n");
					//uhLog("[-] Error :: Service unloaded with errors\n");
					printf("[-] Error :: Service Unload Procedure failed! ::\n");
					break;
				}
			

			}
			else if (c == 'c') {

				// continue.							
				if (ServiceLoadProcedure( ) < 0) {
					uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Service Initialization failed during continue \n");
					//uhLog("[+] Error :: Service Initialization failed\n");
					printf("[-] Error :: Service Load Procedure failed! ::\n");
				}

			}

		}




	}
	__finally {

		if (ServiceUnloadProcedure( ) != 0) {
			printf("[-] Error :: Service Unload Procedure failed! ::\n");
		}

	}

	return ret;

}


void DisplayBanner( ) {

	printf("------------------------------\n");
	printf("----- Uhuru Scan service -----\n");
	printf("------------------------------\n");

	return;
}


int main(int argc, char ** argv) {

	int ret = 0;


	// Only for test purposes (command line) complete test = GUI + driver.
	if ( argc >=2 && strncmp(argv[1],"--testGUI",9) == 0 ){

		DisplayBanner( );

		ret = LaunchCmdLineServiceGUI( );
		if (ret < 0) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;

	}


	// Only for test purposes (command line) complete test = GUI + driver.
	if ( argc >=2 && strncmp(argv[1],"--test",6) == 0 ){

		DisplayBanner( );

		ret = LaunchCmdLineService( );
		if (ret < 0) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;

	}

	

	// Only for test purposes (command line)
	if ( argc >=2 && strncmp(argv[1],"--register",10) == 0 ){

		ret = register_av( );
		if (ret < 0) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;

	}

	// Only for test purposes (command line)
	if ( argc >=2 && strncmp(argv[1],"--crypt",7) == 0 ){

		if (argv[2] == NULL) {
			printf("[-] Error :: --crypt option ::  missing parameter [filename]\n");
			return EXIT_FAILURE;
		}

		ret = verify_file_signature(argv[2],SIGNATURE_FILE);
		if (ret < 0) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;

	}

	// Only for test purposes (command line)
	if ( argc >=3 && strncmp(argv[1],"--quarantine",11) == 0 ){

		ret = MoveFileInQuarantine(argv[2]);
		if (ret < 0) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;

	}
	if ( argc >=3 && strncmp(argv[1],"--restore",9) == 0 ){

		ret = RestoreFileFromQuarantine(argv[2]);
		if (ret < 0) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;

	}


	if ( argc >=2 && strncmp(argv[1],"--updatedb",10) == 0 ){

		DisplayBanner( );

		// 0 : do not reload service (for test)
		// 1 : reload service.
		ret = UpdateModulesDB(0);
		if (ret < 0) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}

	if ( argc >=2 && strncmp(argv[1],"--info",6) == 0 ){
		
		ret = get_av_info();
		if (ret < 0) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}


	
	// command line parameter "--install", install the service.
	if ( argc >=2 && strncmp(argv[1],"--install",9) == 0 ){

		DisplayBanner( );

		ret = ServiceInstall( );
		if (ret < 0) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;

	}

	if ( argc >=2 && strncmp(argv[1],"--installboot",13) == 0 ){

		printf("[TODO] :: InstallBoot not implemented yet!\n" );
		return EXIT_SUCCESS;

	}

	// command line parameter "--uninstall", uninstall the service.
	if ( argc >=2 && strncmp(argv[1],"--uninstall",11) == 0 ){
		DisplayBanner( );
		ret = ServiceRemove( );
		return EXIT_SUCCESS;
	}

	// command line parameter "--remove", delete the service.
	if ( argc >=2 && strncmp(argv[1],"--stop",6) == 0 ){
		ServiceStop();
		return EXIT_SUCCESS;
	}

	if ( argc >=2 && strncmp(argv[1],"--start",7) == 0 ){
		ServiceLaunch( );
		return EXIT_SUCCESS;
	}

	if ( argc >=2 && strncmp(argv[1],"--pause",7) == 0 ){
		ServicePause( );
		return EXIT_SUCCESS;
	}
	if ( argc >=2 && strncmp(argv[1],"--continue",10) == 0 ){
		ServiceContinue( );
		return EXIT_SUCCESS;
	}

	//ServiceLaunchAction( );
	// put this part in ServiceLaunchAction function.
	SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { SVCNAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain }, 
        { NULL, NULL } 
    };


	// This call returs when the service has stopped.
	if (!StartServiceCtrlDispatcher(DispatchTable)) {
		//SvcReportEvent(TEXT("StartServiceCtrlDispatcher"));
		//printf("[i] StartServiceCtrlDispatcher :: %d\n",GetLastError());
	}


	return EXIT_SUCCESS;

}