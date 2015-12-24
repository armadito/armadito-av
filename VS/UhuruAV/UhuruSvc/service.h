#ifndef __SERVICE_H_
#define __SERVICE_H_

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "named_pipe_server.h"

#define SVCNAME TEXT("UhuruSvc")
#define SVCDISPLAY TEXT("Uhuru Scan Service")

#define ROOT_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\eventlog\\Application"
#define APPS_KEY_NAME "UhuruAV"
#define APP_DLL_PATH "\%systemRoot\%\\System32\\uhEventProvider.dll"

#define ROOT_CRASH_KEY_PATH "SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting"
#define ROOT_CRASH_KEY_PATH_LOCAL_DUMPS "SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting\\LocalDumps"
#define ROOT_DRIVER_CRASH_KEY_PATH "SYSTEM\\CurrentControlSet\\Control\\CrashControl"
#define SVC_KEY_NAME "UhuruSvc.exe"

// c:\Users\[username]\AppData\Local\CrashDumps\UhuruSvc
#define DUMP_FOLDER "\%LocalAppData\%\\CrashDumps\\UhuruSvc"

// 0: Custom Dump - 1: Mini dump - 2: Full dump.
#define DUMP_TYPE 1 

// If the real time analysis is enabled or not.
#define REAL_TIME_ENABLED 0 

int ServiceInstall( );
int ServiceRemove( );
VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
void WINAPI ServiceCtrlHandler(DWORD dwCtrl);
void PerformServiceAction( );
void ServiceInit( );
void WINAPI ServiceMain(int argc, char ** argv);
BOOLEAN ServiceLaunchAction( );
void ServiceLaunch( );

void ServiceStop( );


int ServiceLoadProcedure( );
int ServiceUnloadProcedure( );



int RegistryKeysInitialization( );
int DeleteRegistryKeys( );

//
int initClamavDB( );



#endif
