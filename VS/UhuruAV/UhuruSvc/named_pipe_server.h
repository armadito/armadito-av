#ifndef _NAMED_PIPE_SERVER_
#define _NAMED_PIPE_SERVER_

#include <windows.h> 
#include <libuhuru-config.h>
#include <libuhuru/core.h>
#include "utils/json.h"
#include "scan_on_demand.h"

#define PIPE_NAME "\\\\.\\pipe\\uhuruAV_ondemand"
#define SCAN_THREAD_COUNT 3


typedef struct _ONDEMAND_THREAD_CONTEXT {

    //   Threand Handle
    HANDLE   Handle;

    //   Threand Id
    DWORD   ThreadId;

	// Thread Pipe instance
	HANDLE hPipeInst;

    //   We need to remember scan id to know which task to abort.
    LONGLONG  ScanId;
    
    //   A flag that indicates that if this scan thread has received cancel callback from the driver
    UINT  Aborted;

    //   A critical section that synchronize the read/write of ScanId and Aborted.
    CRITICAL_SECTION  Lock;

} ONDEMAND_THREAD_CONTEXT, *PONDEMAND_THREAD_CONTEXT;

typedef struct _ONDEMAND_SCAN_CONTEXT {

    //  Scan thread contexts
    PONDEMAND_THREAD_CONTEXT  MainThreadCtx;

	// Scan Thread Context
	PONDEMAND_THREAD_CONTEXT  ScanThreadCtx;

	// Abort Thread Context
	PONDEMAND_THREAD_CONTEXT  AbortThread;

	UINT Scan_thread_count;
    
    //  The abortion thread handle
    HANDLE   AbortThreadHandle;
    
    //  Finalize flag, set at UserScanFinalize(...)
    UINT  Finalized;
    
    //  Handle of connection port to the filter.
    HANDLE   PipeHandle;

	// Uhuru Structure
	struct uhuru * Uhuru;
    
    //  Completion port for asynchronous message passing    
    HANDLE   Completion;

} ONDEMAND_SCAN_CONTEXT, *PONDEMAND_SCAN_CONTEXT;

struct thread_parameters {
	HANDLE hPipe;
	struct uhuru* uhuru;
};

int TerminateAllScanThreads(PONDEMAND_SCAN_CONTEXT Context);
int Start_IHM_Connection(struct uhuru * uhuru, _Inout_ PONDEMAND_SCAN_CONTEXT Context);
int Close_IHM_Connection(_In_ PONDEMAND_SCAN_CONTEXT Context);
int WINAPI ScanThreadWork(PONDEMAND_SCAN_CONTEXT Context);
int WINAPI MainThreadWork(PONDEMAND_SCAN_CONTEXT Context);

int start_named_pipe_server(struct uhuru* uhuru);
DWORD WINAPI InstanceThread(LPVOID);
int WINAPI InstanceThreadWork(PONDEMAND_SCAN_CONTEXT Context);
VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD, struct new_scan_action* scan);

#endif