#ifndef __SERVICE_H_
#define __SERVICE_H_

#include <Windows.h>


#define SVCNAME TEXT("ArmaditoSvc")
#define SVCDISPLAY TEXT("Armadito antivirus Service")

#define ROOT_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\eventlog\\Application"
#define APPS_KEY_NAME "Armadito-av"
#define APP_DLL_PATH "%systemRoot%\\System32\\a6oEventProvider.dll"

#define ROOT_CRASH_KEY_PATH "SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting"
#define ROOT_CRASH_KEY_PATH_LOCAL_DUMPS "SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting\\LocalDumps"
#define ROOT_DRIVER_CRASH_KEY_PATH "SYSTEM\\CurrentControlSet\\Control\\CrashControl"
#define SVC_KEY_NAME "ArmaditoSvc.exe"

// c:\Users\[username]\AppData\Local\CrashDumps\ArmaditoSvc
#define DUMP_FOLDER "%LocalAppData%\\CrashDumps\\ArmaditoSvc"

// 0: Custom Dump - 1: Mini dump - 2: Full dump.
#define DUMP_TYPE 1

typedef enum start_mode {
	SVC_MODE = 1,
	GUI_ONLY = 2
}start_mode;



int ServiceInstall(DWORD startType);
int ServiceRemove( );
VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
void WINAPI ServiceCtrlHandler(DWORD dwCtrl);
void PerformServiceAction( );
void ServiceInit( );
void WINAPI ServiceMain(int argc, char ** argv);
BOOLEAN ServiceLaunchAction( );
void ServiceLaunch( );

void ServiceStop( );
int ServicePause( );
int ServiceContinue();


int ServiceLoadProcedure(start_mode mode);
//int ServiceLoadProcedure_cmd(cmd_mode mode);
int ServiceUnloadProcedure( );



int RegistryKeysInitialization( );
int DeleteRegistryKeys( );




#endif
