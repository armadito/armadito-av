#ifndef __SCAN_ON_DEMAND_H__
#define __SCAN_ON_DEMAND_H__

#include "structs.h"

#define SVC_IPC_PATH "\\\\.\\pipe\\Armadito_ondemand"
#define UI_IPC_PATH "\\\\.\\pipe\\Armadito-UI"
#define SCAN_THREAD_COUNT 3
#define BUFSIZE 512


/* Functions prototypes*/
int Start_IHM_Connection(_Inout_ PGLOBAL_SCAN_CONTEXT Context);
int WINAPI MainThreadWork(PGLOBAL_SCAN_CONTEXT Context);
int WINAPI ScanThreadWork(PGLOBAL_SCAN_CONTEXT Context);
int Close_IHM_Connection(_In_ PGLOBAL_SCAN_CONTEXT Context);
int TerminateAllScanThreads(PGLOBAL_SCAN_CONTEXT Context);


#endif