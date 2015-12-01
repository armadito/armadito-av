#ifndef __SERVICE_H_
#define __SERVICE_H_

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#define SVCNAME TEXT("UhuruSvc")
#define SVCDISPLAY TEXT("Uhuru Scan Service")

#define ROOT_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\eventlog\\Application"
#define APPS_KEY_NAME "UhuruAV"
#define APP_DLL_PATH "\%systemRoot\%\\System32\\uhEventProvider.dll"


int ServiceInstall( );
int ServiceRemove( );
VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
void WINAPI ServiceCtrlHandler(DWORD dwCtrl);
void LaunchServiceAction( );
void ServiceInit( );
void WINAPI ServiceMain(int argc, char ** argv);
BOOLEAN ServiceLaunchAction( );
void ServiceLaunch( );
void ServiceStop( );

int RegistryKeysInitialization( );
int DeleteRegistryKeys( );

//
int initClamavDB( );



#endif
