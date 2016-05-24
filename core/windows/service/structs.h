#ifndef __UH_STRUCTS_H__
#define __UH_STRUCTS_H__

#include <Windows.h>


/* On-demand structures */
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
    
    //  Completion port for asynchronous message passing    
    HANDLE   Completion;

} ONDEMAND_SCAN_CONTEXT, *PONDEMAND_SCAN_CONTEXT;

/* On-access structures */
typedef struct _ONACCESS_THREAD_CONTEXT {

	//   Threand Handle
	HANDLE   Handle;

	//   Threand Id
	DWORD   ThreadId;

	//   We need to remember scan id to know which task to abort.
	LONGLONG  ScanId;

	//   A flag that indicates that if this scan thread has received cancel callback from the driver
	UINT  Aborted;

	//   A critical section that synchronize the read/write of ScanId and Aborted.
	CRITICAL_SECTION  Lock;

} ONACCESS_THREAD_CONTEXT, *PONACCESS_THREAD_CONTEXT;

typedef struct _ONACCESS_SCAN_CONTEXT {

    //  Scan thread contexts
    PONACCESS_THREAD_CONTEXT ScanThreadCtxes;
    
    //  The abortion thread handle
    HANDLE AbortThreadHandle;
    
    //  Finalize flag, set at UserScanFinalize(...)
    UINT  Finalized;
    
    //  Handle of connection port to the filter.
    HANDLE   ConnectionPort;
    
    //  Completion port for asynchronous message passing
    HANDLE   Completion;

} ONACCESS_SCAN_CONTEXT, *PONACCESS_SCAN_CONTEXT;

typedef struct _GLOBAL_SCAN_CONTEXT {

	//  On demand Scan context
	PONDEMAND_SCAN_CONTEXT onDemandCtx;
    
	//  On access Scan context    	
	PONACCESS_SCAN_CONTEXT onAccessCtx;
    
	// Armadito Structure.
	struct armadito * armadito;

	// Flag for finish order.
	int FinalizeAll;

} GLOBAL_SCAN_CONTEXT, *PGLOBAL_SCAN_CONTEXT;


#endif
