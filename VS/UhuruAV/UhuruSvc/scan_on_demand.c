#include <Windows.h>
#include <sddl.h>
#include <stdio.h>
#include "scan_on_demand.h"

/*
	int do_apply_conf( )
	This function apply changes made in configuration (...)

*/
int do_apply_conf( ) {
	return 0;
}

int CreatePipeSecurityAttributes(SECURITY_ATTRIBUTES * pSa) {

	int ret = 0;
	ULONG sdSize = 0;
	PSECURITY_DESCRIPTOR pSd = NULL;


	// Define the SDDL (Security Descriptor Definition Language) for the DACL.

	// Security Descriptor String Format.
	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa379567%28v=vs.85%29.aspx
	

	// ACE strings :: D:dacl_flags(string_ace1)(string_ace2)... (string_acen)
	// ace_type;ace_flags;rights;object_guid;inherit_object_guid;account_sid;(resource_attribute)
	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa374928%28v=vs.85%29.aspx

	// SID strings :: for account sid.
	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa379602%28v=vs.85%29.aspx

	/*// This code set the following access:
	LPCWSTR szSDDL = L"D:"       // Discretionary ACL
		L"(A;OICI;GRGW;;;AU)"   // Allow read/write to authenticated users
		L"(D;OICI;GA;;;AN)"		// Deny access for non-authenticated users (Anonymous logon)
		L"(A;OICI;GA;;;BA)";    // Allow full control to administrators
	*/
	LPCSTR szSDDL = "D:"       // Discretionary ACL
		"(A;OICI;GRGW;;;AU)"   // Allow read/write to authenticated users
		"(D;OICI;GA;;;AN)"		// Deny access for non-authenticated users (Anonymous logon)
		"(A;OICI;GA;;;BA)";    // Allow full control to administrators
		

	if (pSa == NULL) {
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," CreatePipeSecurityAttributes :: NUll pointer for SecurityAttributes !\n" );
		return -1;
	}

	__try {

		// Convert a String security Descriptor  to a valid functional security desciptor.
		if (ConvertStringSecurityDescriptorToSecurityDescriptorA(szSDDL, SDDL_REVISION_1, &pSd, &sdSize) == FALSE) {			
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," CreatePipeSecurityAttributes :: ConvertStringSecurityDescriptorToSecurityDescriptor failed :: GLE=%d\n", GetLastError( ));
			ret = -2;
			__leave;
		}
		
		pSa->lpSecurityDescriptor = pSd;
		pSa->nLength = sizeof(SECURITY_ATTRIBUTES);
		pSa->bInheritHandle = FALSE;


	}
	__finally{		

	}


	return ret;
}

int WINAPI ScanThreadWork(PGLOBAL_SCAN_CONTEXT Context) {

	int ret = 0;	
	int i = 0, index = 0, count =0;
	DWORD threadId = 0;
	PONDEMAND_THREAD_CONTEXT threadCtx = NULL;	
	HANDLE hHeap = GetProcessHeap();
	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;	

	struct uhuru_json_handler * json_handler = NULL;
	enum uhuru_json_status status = JSON_OK ;
	char * request = NULL;
	int req_len = 0;
	char * response = NULL;
	int resp_len = 0;

	PVOID OldValue = NULL;
	

	if (Context == NULL) {
		printf("[-] Error :: ScanThreadWork :: NULL Context\n" );
		return -1;
	}


	// Get thread context by ID.
	threadId = GetCurrentThreadId( );
	printf("[+] Debug :: ScanThreadWork :: Thread Id = %d \n",threadId);

	for (i = 0; i < SCAN_THREAD_COUNT; i++) {		
		if (threadId == Context->onDemandCtx->ScanThreadCtx[i].ThreadId) {
			threadCtx = &Context->onDemandCtx->ScanThreadCtx[i];
			index = i;
			break;
		}
	}

	if (threadCtx == NULL) {
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " ScanThreadWork :: Thread context not found!\n");
		printf("[-] Error :: ScanThreadWork :: Thread Context not found\n");
		return -2;
	}

	// Disables file system redirection for the calling thread.
	if (Wow64DisableWow64FsRedirection(&OldValue) == FALSE) {
		return S_FALSE;
	}

	__try {


		if (Context->onDemandCtx->Finalized) {
			printf("[+] Debug :: ScanThreadWork :: [%d] :: Finalizing thread execution...\n",threadCtx->ThreadId);
			__leave;
		}
		
		if (threadCtx->hPipeInst == NULL) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " ScanThreadWork :: Thread pipe instance is invalid (NULL) !\n");
			printf("[-] Error :: ScanThreadWork :: Thread pipe instance is NULL\n");
			ret = -3;
			__leave;
		}		
		
		// allocate request buffer
		request = (char*)calloc(BUFSIZE,sizeof(char));
		if (request == NULL) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " ScanThreadWork :: Request Buffer Allocation failed! :: GLE= %d \n",GetLastError());
			printf("[-] Error :: ScanThreadWork :: Request buffer Allocation failed with error :: %d \n",GetLastError());
			ret = -4;
			__leave;
		}		


		while (TRUE) {

			if (Context->onDemandCtx->Finalized) {
				printf("[+] Debug :: ScanThreadWork :: [%d] :: Finalizing thread execution...\n",threadCtx->ThreadId);
				__leave;
			}

			// Read GUI request from the pipe.
			if ((ReadFile(threadCtx->hPipeInst, request, BUFSIZE,&cbBytesRead,NULL) == FALSE) || cbBytesRead <=0) {
				printf("[-] Error :: ScanThreadWork :: Read in pipe failed with error :: %d \n",GetLastError());
				ret = -6;
				__leave;
			}
			req_len = cbBytesRead;
			printf("[+] Debug :: ScanThreadWork :: len = %d ::  GUI request = %s ::\n",req_len, request);
			
			// intialize json_handler.
			json_handler =  uhuru_json_handler_new(Context->uhuru);
			if (json_handler == NULL) {
				printf("[-] Error :: ScanThreadWork :: uhuru_json_handler_new failed! \n");
				ret = -7;
				__leave;
			}

			//json_handler = (struct uhuru_json_handler *)calloc(1,sizeof(struct uhuru_json_handler));

			status = uhuru_json_handler_get_response(json_handler, request, req_len, &response, &resp_len);
			if (status != JSON_OK) {
				printf("[-] Error :: ScanThreadWork :: uhuru json handler get response failed :: status = %d\n",status);
				ret = -8;
				break;
			}

			printf("[+] Debug :: ScanThreadWork :: resp_len= %d :: response= %s\n",resp_len, response);

			// write answer to GUI.
			if ( (WriteFile(threadCtx->hPipeInst, response, resp_len, &cbWritten, NULL) == FALSE ) || cbWritten <= 0) {
				printf("[-] Error :: ScanThreadWork :: Write in pipe failed with error :: %d \n",GetLastError());
				ret = -9;
				break;
			}

			// execute request callback.
			uhuru_json_handler_process(json_handler);



		} // end of while

		// Flush the pipe to allow the client to read the pipe's contents 
		// before disconnecting. Then disconnect the pipe, and close the 
		// handle to this pipe instance.
		//FlushFileBuffers(threadCtx->hPipeInst);
		//DisconnectNamedPipe(threadCtx->hPipeInst);
		//CloseHandle(Context->PipeHandle);


	}
	__finally {

		// Re enable FS redirection for this thread.
		if (Wow64RevertWow64FsRedirection(OldValue) == FALSE ){
			printf("[-] Error :: ScanThreadWork :: can't revert file system redirection !\n");
		}

		FlushFileBuffers(threadCtx->hPipeInst);
		DisconnectNamedPipe(threadCtx->hPipeInst);

		
		if (request != NULL) {
			free(request);
			request = NULL;
		}

		if (response != NULL) {
			free(response);
			response = NULL;
		}

		if (json_handler != NULL) {
			uhuru_json_handler_free(json_handler);
			//free(json_handler);
			json_handler = NULL;
		}					

		// Close the pipe instance.
		if (!CloseHandle(threadCtx->hPipeInst)) {
			printf("[-] Error :: ScanThreadWork :: [%d] :: CloseHandle failed with error :: %d\n",index,GetLastError());
		}

		// remove the thread from the scan thread pool.		
		Context->onDemandCtx->ScanThreadCtx[index].Aborted = FALSE ;
		Context->onDemandCtx->ScanThreadCtx[index].Handle = NULL;
		Context->onDemandCtx->ScanThreadCtx[index].hPipeInst = NULL;
		Context->onDemandCtx->ScanThreadCtx[index].ScanId = 0;
		Context->onDemandCtx->ScanThreadCtx[index].ThreadId = 0;

		// decrease the thread count.
		Context->onDemandCtx->Scan_thread_count --;

	}
	

	return ret;
}

int WINAPI MainThreadWork(PGLOBAL_SCAN_CONTEXT Context) {

	int ret = 0;
	BOOL bConnected = FALSE;
	HANDLE hPipe = NULL;
	ONDEMAND_THREAD_CONTEXT threadCtx = {0};
	SECURITY_ATTRIBUTES securityAttributes = {0};
	int i = 0, index = 0;

	if (Context == NULL) {
		printf("[-] Error :: ScanThreadWork :: NULL Context\n" );
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Request Buffer Allocation failed! :: GLE= %d \n",GetLastError());
		return -1;
	}	

	// 
	__try {

		// Allocate memory for Scan threads contexts.
		Context->onDemandCtx->ScanThreadCtx = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ONDEMAND_THREAD_CONTEXT) * SCAN_THREAD_COUNT);
		if (Context->onDemandCtx->ScanThreadCtx == NULL) {    
			ret = -2;
			__leave;
		}

		/*for (i = 0; i < USER_SCAN_THREAD_COUNT; i ++) {
			printf("[+] Debug :: MainThreadWork :: ThreadId = %d - ThreadHandle  = %d\n", Context->ScanThreadCtx[i].ThreadId, Context->ScanThreadCtx[i].Handle );
		}*/
		
		// Create and Initialize security descriptor
		if (CreatePipeSecurityAttributes(&securityAttributes) < 0) {
			printf("[-] Error :: MainThreadWork :: CreateSecurityAttributtes() failed!\n");
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," MainThreadWork :: CreateSecurityAttributtes() failed!\n");
			ret = -2;
			__leave;
		}

		while (TRUE) {

			// Termintate instruction.
			if (Context->onDemandCtx->Finalized == TRUE) {
				printf("[+] Debug :: MainThreadWork :: Terminating main thread!\n");
				__leave;
			}

			// If the limit of thread is reached.
			if (Context->onDemandCtx->Scan_thread_count == SCAN_THREAD_COUNT) {
				//printf("[w] Warning :: MainThreadWork :: Scan Thread count limit is reached :: [%d] !\n", Context->Scan_thread_count);
				continue;
			}

			//. create the pipe instance.
			hPipe = CreateNamedPipeA(
				SVC_IPC_PATH,             // pipe name 
				PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,       // read/write access 
				PIPE_TYPE_MESSAGE |       // message type pipe 
				PIPE_READMODE_MESSAGE |   // message-read mode 
				PIPE_WAIT,                // blocking mode 
				PIPE_UNLIMITED_INSTANCES, // max. instances  
				BUFSIZE,                  // output buffer size 
				BUFSIZE,                  // input buffer size 
				0,                        // client time-out
				&securityAttributes);                    // default security attribute

			if (hPipe == INVALID_HANDLE_VALUE) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " MainThreadWork :: Pipe creation failed! :: GLE= %d \n",GetLastError());
				printf("[-] Error :: MainThreadWork :: CreateNamedPipeA failed :: %d\n",GetLastError());
				ret = -3;
				__leave;
			}

			// Create a scan/abort thread.
			printf("[+] Debug :: MainThreadWork :: Waiting for client connection...\n");
			bConnected = ConnectNamedPipe(hPipe, NULL) ?
			TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

			if (!bConnected) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " MainThreadWork :: Pipe connection failed! :: GLE= %d \n",GetLastError());
				printf("[-] Error :: MainThreadWork :: ConnectNamedPipe failed :: %d\n",GetLastError());
				ret = -4;
				__leave;
			}
		
			printf("[+] Debug :: MainThreadWork :: Client connected to the Pipe\n");


			// Find the index to store the new thread in the pool.
			for (i = 0; i < SCAN_THREAD_COUNT; i++) {
				if (Context->onDemandCtx->ScanThreadCtx[i].Handle != NULL) {
					index = i;
					break;
				}
			}
			
			Context->onDemandCtx->ScanThreadCtx[index].hPipeInst = hPipe;
			Context->onDemandCtx->ScanThreadCtx[index].ScanId = 0;
			Context->onDemandCtx->ScanThreadCtx[index].Handle = CreateThread(
					NULL,              // no security attribute 
					0,                 // default stack size 
					ScanThreadWork,    // thread proc
					Context,			// thread parameter 
					CREATE_SUSPENDED,                 // not suspended 
					&Context->onDemandCtx->ScanThreadCtx[index].ThreadId);      // returns thread ID

			if (Context->onDemandCtx->ScanThreadCtx[index].Handle == NULL) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " MainThreadWork :: Scan Thread creation failed! :: GLE= %d \n",GetLastError());
				printf("[-] Error :: MainThreadWork :: CreateThread failed :: %d\n",GetLastError());
				ret = -5;
				__leave;
			}

			if (ResumeThread(Context->onDemandCtx->ScanThreadCtx[index].Handle) == -1) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " MainThreadWork :: Resume Scan thread failed! :: GLE= %d \n",GetLastError());
				printf("[-] Error :: MainThreadWork :: ResumeThread failed :: %d\n",GetLastError());
				ret = -6;

				__leave;
			}
		
			//Context->ScanThreadCtx[i] = threadCtx;		
			Context->onDemandCtx->Scan_thread_count++;
			i = Context->onDemandCtx->Scan_thread_count;
			printf("[+] Debug :: MainThreadWork :: Scan Thread [%d] created successfully! :: index = %d :: count = %d\n",Context->onDemandCtx->ScanThreadCtx[i-1].ThreadId,index, Context->onDemandCtx->Scan_thread_count);
			

		}

		

	}
	__finally {

		if (ret<0) {

			// Terminate all scan threads.			
			TerminateAllScanThreads(Context);			

			// Free allocated memory
			if (Context->onDemandCtx->ScanThreadCtx != NULL) {
				if (!(HeapFree(GetProcessHeap( ), 0, Context->onDemandCtx->ScanThreadCtx)) ) {
					printf("[-] Error :: MainThreadWork :: HeapFree failed with error :: %d\n",GetLastError());
				}
			}
			
			// Free allocated memory
			if (Context->onDemandCtx->MainThreadCtx != NULL) {
				if (!(HeapFree(GetProcessHeap( ), 0, Context->onDemandCtx->MainThreadCtx)) ) {
					printf("[-] Error :: MainThreadWork :: HeapFree failed with error :: %d\n",GetLastError());
				}
			}
			Context->onDemandCtx->MainThreadCtx = NULL;

		}
	}	

	return ret;
}

int Start_IHM_Connection(_Inout_ PGLOBAL_SCAN_CONTEXT Context) {

	int ret = 0;
	HANDLE hPipe = NULL;
	HANDLE hScanThread = NULL;
	HANDLE hAbortThread = NULL;
	DWORD dwThreadId = 0;
	PONDEMAND_THREAD_CONTEXT threadCtx = NULL;
	PONDEMAND_THREAD_CONTEXT mainThreadCtx = NULL;
	BOOL bConnected = FALSE;



	// Create main thread
	// In main thread :: create thread named pipe
	// In main thread :: wait for connection.
	// In main thread :: create a scan thread for each connection.

	if (Context->uhuru == NULL) {
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR,"[-] Error :: Start_IHM_Connection :: NULL uhuru structure!\n");
		return -1;
	}

	__try {

		// on-access context allocation.
		Context->onDemandCtx = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ONDEMAND_SCAN_CONTEXT) *1);
		if (Context->onDemandCtx == NULL) {    
			ret = -2;
			__leave;
		}
		//ZeroMemory(Context->onDemandCtx, sizeof(ONDEMAND_SCAN_CONTEXT));


		// Create and Initialize main thread contexts. (containing threadID, handle to the thread, 
		mainThreadCtx = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ONDEMAND_THREAD_CONTEXT) * 1);
		if (mainThreadCtx == NULL) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Main Thread heap allocation failed! :: GLE= %d \n",GetLastError());
			printf("[-] Error :: Start_IHM_Connection :: HeapAlloc failed! with errror :: %d\n", GetLastError());
			ret = -2;			
			__leave;
		}
		//ZeroMemory(mainThreadCtx, sizeof(ONDEMAND_THREAD_CONTEXT) * 1);

		mainThreadCtx->Handle = CreateThread(
				NULL,              // no security attribute 
				0,                 // default stack size 
				MainThreadWork,    // thread proc
				Context,			// thread parameter 
				CREATE_SUSPENDED,  // not suspended 
				&dwThreadId);      // returns thread ID

		if (mainThreadCtx->Handle == INVALID_HANDLE_VALUE) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Main thread creation failed! :: GLE= %d \n",GetLastError());
			printf("[-] Error :: start_IHM_Connection :: CreateThread failed :: %d\n",GetLastError());
			ret = -3;
			__leave;
		}
				
		mainThreadCtx->ThreadId = dwThreadId;
		Context->onDemandCtx->MainThreadCtx = mainThreadCtx;
		Context->onDemandCtx->Finalized = FALSE;
		Context->onDemandCtx->Scan_thread_count = 0;
		
		printf("[+] Debug :: start_IHM_Connection :: Main Thread [%d] created successfully!\n",mainThreadCtx->ThreadId);

		// Resuming the main thread.
		if (ResumeThread(Context->onDemandCtx->MainThreadCtx->Handle) == -1) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Resuming main thread failed! :: GLE= %d \n",GetLastError());
			printf("[-] Error :: start_IHM_Connection :: ResumeThread failed :: %d\n",GetLastError());
			ret = -4;
			__leave;
		}


	}
	__finally {

		// on failure.
		if (ret < 0) {

			if (Context->onDemandCtx->MainThreadCtx->Handle != NULL && Context->onDemandCtx->MainThreadCtx->Handle != INVALID_HANDLE_VALUE ) {
				CloseHandle(Context->onDemandCtx->MainThreadCtx->Handle);
				Context->onDemandCtx->ScanThreadCtx->Handle = NULL;
			}
			// Free allocated memory
			if (Context->onDemandCtx->MainThreadCtx != NULL) {
				if (!(HeapFree(GetProcessHeap( ), 0, Context->onDemandCtx->MainThreadCtx)) ) {
					printf("[-] Error :: start_IHM_Connection :: HeapFree failed with error :: %d\n",GetLastError());
				}
				Context->onDemandCtx->MainThreadCtx = NULL;
			}

			if (Context->onDemandCtx != NULL) {
				HeapFree(GetProcessHeap( ), 0, Context->onDemandCtx);
				Context->onDemandCtx = NULL;
			}

		}

	}

	return ret;


}

int Close_IHM_Connection(_In_ PGLOBAL_SCAN_CONTEXT Context ) {

	int ret = 0;
	HANDLE hPipe = NULL;

	if (Context == NULL) {
		printf("[-] Error :: close_IHM_Connection :: NULL Context\n" );
		return -1;
	}

	__try {

		if (Context->onDemandCtx->PipeHandle != NULL && Context->onDemandCtx->PipeHandle != INVALID_HANDLE_VALUE ) {
			if (CloseHandle(Context->onDemandCtx->PipeHandle)) {
				printf("[+] Debug :: close_IHM_Connection :: Pipe closed successfully!\n");
				Context->onDemandCtx->PipeHandle = NULL;
			}
			else {
				printf("[-] Error :: close_IHM_Connection :: CloseHandle failed with error :: %d\n",GetLastError());
			}
			
		}

		// Terminate all scan threads
		TerminateAllScanThreads(Context);

		printf("[+] Debug :: close_IHM_Connection :: Terminating Main thread...\n");
		if (Context->onDemandCtx->MainThreadCtx != NULL && Context->onDemandCtx->MainThreadCtx->Handle != INVALID_HANDLE_VALUE) {

			// Terminate the main thread.
			if (!TerminateThread(Context->onDemandCtx->MainThreadCtx->Handle, 0)) {
				printf("[-] Error :: close_IHM_Connection :: TerminateThread failed with error :: %d\n",GetLastError());
			}
			printf("[+] Debug :: close_IHM_Connection :: Terminating Main thread...[OK]\n");

			/*if (CloseHandle(Context->MainThreadCtx->Handle)) {
				printf("[+] Debug :: close_IHM_Connection :: Pipe closed successfully!\n");
				Context->PipeHandle = NULL;
			}
			else
				printf("[-] Error :: close_IHM_Connection :: CloseHandle failed with error :: %d\n",GetLastError());
				*/
		}
		printf("[+] Debug :: close_IHM_Connection :: Main thread...[OK]\n");


		// Free allocated memory
		if (Context->onDemandCtx->MainThreadCtx != NULL) {
			if (!(HeapFree(GetProcessHeap( ), 0, Context->onDemandCtx->MainThreadCtx)) ) {
				printf("[-] Error :: start_IHM_Connection :: HeapFree failed with error :: %d\n",GetLastError());
			}
			Context->onDemandCtx->MainThreadCtx = NULL;
		}


	}
	__finally {

	}

	return ret;

}

/*
	function :: TerminateAllThreads(PONDEMAND_SCAN_CONTEXT Context)
	This function performs the following actions:
		- send a terminate signal to all the scan threads.
		- close all scan threads handles
		- terminate the main thread and close its handle
		- free allocated memories.
*/
int TerminateAllScanThreads(PGLOBAL_SCAN_CONTEXT Context) {

	int ret = 0;
	HANDLE hScanThreads[SCAN_THREAD_COUNT] = {0};
	int i = 0;
	int count = 0;

	Context->onDemandCtx->Finalized = TRUE;

	if (Context->onDemandCtx->ScanThreadCtx == NULL)
		return -1;

	for (i = 0; i < SCAN_THREAD_COUNT; i++) {

		if (Context->onDemandCtx->ScanThreadCtx[i].Handle != INVALID_HANDLE_VALUE && Context->onDemandCtx->ScanThreadCtx[i].Handle != NULL) {
			hScanThreads[i] = Context->onDemandCtx->ScanThreadCtx[i].Handle;
			CloseHandle(Context->onDemandCtx->ScanThreadCtx[i].hPipeInst);
			count++;
		}

		/*if (Context->ScanThreadCtx[i].Handle == INVALID_HANDLE_VALUE || Context->ScanThreadCtx[i].Handle == NULL ) {
			printf("[-] Error :: TerminateAllThreads :: NULL Thread Handle\n");
			break;
		}*/		
	}

	printf("[+] Debug :: TerminateAllScanThreads :: Waiting for thread terminating...\n");
	if (count == 0) {
		printf("[+] Debug :: TerminateAllScanThreads :: No thread to terminate\n");
	}

	if (count > 0 && WaitForMultipleObjects(SCAN_THREAD_COUNT, hScanThreads, TRUE, INFINITE) == WAIT_FAILED) {
		printf("[w] Warning :: TerminateAllScanThreads :: WaitForMultipleObjects failed with error :: %d  \n", GetLastError( ));
	}

	printf("[+] Debug :: TerminateAllScanThreads :: All thread terminated successfully!\n");

	for (i = 0; i < SCAN_THREAD_COUNT; i++) {		
		hScanThreads[i] = Context->onDemandCtx->ScanThreadCtx[i].Handle = NULL;
	}

	if (Context->onDemandCtx->ScanThreadCtx != NULL) {
		HeapFree(GetProcessHeap(), 0, Context->onDemandCtx->ScanThreadCtx);
		Context->onDemandCtx->ScanThreadCtx = NULL;
	}
		
	printf("[+] Debug :: TerminateAllScanThreads :: exit!\n");

	return ret;
}

