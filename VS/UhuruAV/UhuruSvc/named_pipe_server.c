#include "named_pipe_server.h"
#include "named_pipe_client.h"


#define BUFSIZE 512

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
			printf("\n[-] Error ::  CreatePipeSecurityAttributes :: ConvertStringSecurityDescriptorToSecurityDescriptor failed :: GLE=%d\n", GetLastError( ));
			//uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," CreatePipeSecurityAttributes :: ConvertStringSecurityDescriptorToSecurityDescriptor failed :: GLE=%d\n", GetLastError( ));
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

/*
	function :: TerminateAllThreads(PONDEMAND_SCAN_CONTEXT Context)
	This function performs the following actions:
		- send a terminate signal to all the scan threads.
		- close all scan threads handles
		- terminate the main thread and close its handle
		- free allocated memories.
*/
int TerminateAllScanThreads(PONDEMAND_SCAN_CONTEXT Context) {

	int ret = 0;
	HANDLE hScanThreads[SCAN_THREAD_COUNT] = {0};
	int i = 0;
	int count = 0;

	Context->Finalized = TRUE;

	if (Context->ScanThreadCtx == NULL)
		return -1;

	for (i = 0; i < SCAN_THREAD_COUNT; i++) {

		if (Context->ScanThreadCtx[i].Handle != INVALID_HANDLE_VALUE && Context->ScanThreadCtx[i].Handle != NULL) {
			hScanThreads[i] = Context->ScanThreadCtx[i].Handle;
			CloseHandle(Context->ScanThreadCtx[i].hPipeInst);
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
		hScanThreads[i] = Context->ScanThreadCtx[i].Handle = NULL;
	}

	if (Context->ScanThreadCtx != NULL) {
		HeapFree(GetProcessHeap(), 0, Context->ScanThreadCtx);
		Context->ScanThreadCtx = NULL;
	}
		
	printf("[+] Debug :: TerminateAllScanThreads :: exit!\n");

	return ret;
}

int WINAPI ScanThreadWork(PONDEMAND_SCAN_CONTEXT Context) {

	int ret = 0;
	BOOL bConnected = FALSE;
	HANDLE hPipe = NULL;
	int i = 0, index = 0, count =0;
	DWORD threadId = 0;
	PONDEMAND_THREAD_CONTEXT threadCtx = NULL;
	
	HANDLE hHeap = GetProcessHeap();
	TCHAR* pchRequest = NULL;
	TCHAR* pchReply = NULL;
	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	struct new_scan_action* scan = NULL;

	if (Context == NULL) {
		printf("[-] Error :: ScanThreadWork :: NULL Context\n" );
		return -1;
	}


	// Get thread context by ID.
	threadId = GetCurrentThreadId( );
	printf("[+] Debug :: ScanThreadWork :: Thread Id = %d \n",threadId);

	for (i = 0; i < SCAN_THREAD_COUNT; i++) {		
		if (threadId == Context->ScanThreadCtx[i].ThreadId) {
			threadCtx = &Context->ScanThreadCtx[i];
			index = i;
			break;
		}
	}

	if (threadCtx == NULL) {
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " ScanThreadWork :: Thread context not found!\n");
		printf("[-] Error :: ScanThreadWork :: Thread Context not found\n");
		return -2;
	}


	__try {


		if (Context->Finalized) {
			printf("[+] Debug :: ScanThreadWork :: [%d] :: Finalizing thread execution...\n",threadCtx->ThreadId);
			__leave;
		}
		
		if (threadCtx->hPipeInst == NULL) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " ScanThreadWork :: Thread pipe instance is invalid (NULL) !\n");
			printf("[-] Error :: ScanThreadWork :: Thread pipe instance is NULL\n");
			ret = -3;
			__leave;
		}

		pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));
		if (pchRequest == NULL) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " ScanThreadWork :: Request Buffer Allocation failed! :: GLE= %d \n",GetLastError());
			printf("[-] Error :: ScanThreadWork :: Request buffer Allocation failed with error :: %d \n",GetLastError());
			ret = -4;
			__leave;
		}		

		pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));
		if (pchReply == NULL){
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " ScanThreadWork :: Reply Buffer Allocation failed! :: GLE= %d \n",GetLastError());
			printf("[-] Error :: ScanThreadWork :: Reply buffer Allocation failed with error :: %d \n",GetLastError());
			ret = -5;
			__leave;
		}

		while (TRUE) {

			if (Context->Finalized) {
				printf("[+] Debug :: ScanThreadWork :: [%d] :: Finalizing thread execution...\n",threadCtx->ThreadId);
				__leave;
			}

			// Read client requests from the pipe. This simplistic code only allows messages
			// up to BUFSIZE characters in length.
			fSuccess = ReadFile(
				threadCtx->hPipeInst,        // handle to pipe 
				pchRequest,    // buffer to receive data 
				BUFSIZE*sizeof(TCHAR), // size of buffer 
				&cbBytesRead, // number of bytes read 
				NULL);        // not overlapped I/O 

			if (!fSuccess || cbBytesRead == 0) {
				printf("[-] Error :: ScanThreadWork :: Read in pipe failed with error :: %d \n",GetLastError());
				ret = -6;
				break;
			}

			scan = (struct new_scan_action*)malloc(sizeof(struct new_scan_action));
			// To avoid random data on buffer after data read
			pchRequest[cbBytesRead] = '\0';

			// Process the incoming message. Json parsing -> fill scan variable
			GetAnswerToRequest(pchRequest, pchReply, &cbReplyBytes, scan);

			// Write the reply to the pipe. 
			fSuccess = WriteFile(
				threadCtx->hPipeInst,        // handle to pipe 
				pchReply,     // buffer to write from 
				cbReplyBytes, // number of bytes to write 
				&cbWritten,   // number of bytes written 
				NULL);        // not overlapped I/O

			if (!fSuccess || cbReplyBytes != cbWritten) {
				printf("[-] Error :: ScanThreadWork :: Write in pipe failed with error :: %d \n",GetLastError());
				ret = -7;
				break;
			}

			// Step 4 -- Start Scan here if message already sent
			if (strcmp(scan->scan_action, "new_scan") == 0){
				start_new_scan(scan, Context->Uhuru);
			}
			else if (strcmp(scan->scan_action, "cancel") == 0){
				cancel_current_scan(scan, Context->Uhuru);
			}

		} // end of while

		// Flush the pipe to allow the client to read the pipe's contents 
		// before disconnecting. Then disconnect the pipe, and close the 
		// handle to this pipe instance.
		FlushFileBuffers(threadCtx->hPipeInst);
		DisconnectNamedPipe(threadCtx->hPipeInst);
		//CloseHandle(Context->PipeHandle);


	}
	__finally {

		// Free allocated memory.
		if (pchReply != NULL)
			HeapFree(hHeap, 0, pchReply);

		if (pchRequest != NULL) 
			HeapFree(hHeap, 0, pchRequest);

		if (scan != NULL) {
			free(scan);
			scan = NULL;
		}		

		// Close the pipe instance.
		if (!CloseHandle(threadCtx->hPipeInst)) {
			printf("[-] Error :: ScanThreadWork :: [%d] :: CloseHandle failed with error :: %d\n",GetLastError());
		}

		// remove the thread from the scan thread pool.		
		Context->ScanThreadCtx[index].Aborted = FALSE ;
		Context->ScanThreadCtx[index].Handle = NULL;
		Context->ScanThreadCtx[index].hPipeInst = NULL;
		Context->ScanThreadCtx[index].ScanId = 0;
		Context->ScanThreadCtx[index].ThreadId = 0;

		// decrease the thread count.
		Context->Scan_thread_count --;

	}
	

	return ret;
}

int WINAPI MainThreadWork(PONDEMAND_SCAN_CONTEXT Context) {

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
		Context->ScanThreadCtx = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ONDEMAND_THREAD_CONTEXT) * SCAN_THREAD_COUNT);
		if (Context->ScanThreadCtx == NULL) {    
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
			if (Context->Finalized == TRUE) {
				printf("[+] Debug :: MainThreadWork :: Terminating main thread!\n");
				__leave;
			}

			// If the limit of thread is reached.
			if (Context->Scan_thread_count == SCAN_THREAD_COUNT) {
				//printf("[w] Warning :: MainThreadWork :: Scan Thread count limit is reached :: [%d] !\n", Context->Scan_thread_count);
				continue;
			}

			//. create the pipe instance.
			hPipe = CreateNamedPipeA(
				PIPE_NAME,             // pipe name 
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
				if (Context->ScanThreadCtx[i].Handle != NULL) {
					index = i;
					break;
				}
			}
			
			Context->ScanThreadCtx[index].hPipeInst = hPipe;
			Context->ScanThreadCtx[index].ScanId = 0;
			Context->ScanThreadCtx[index].Handle = CreateThread(
					NULL,              // no security attribute 
					0,                 // default stack size 
					ScanThreadWork,    // thread proc
					Context,			// thread parameter 
					CREATE_SUSPENDED,                 // not suspended 
					&Context->ScanThreadCtx[index].ThreadId);      // returns thread ID

			if (Context->ScanThreadCtx[index].Handle == NULL) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " MainThreadWork :: Scan Thread creation failed! :: GLE= %d \n",GetLastError());
				printf("[-] Error :: MainThreadWork :: CreateThread failed :: %d\n",GetLastError());
				ret = -5;
				__leave;
			}

			if (ResumeThread(Context->ScanThreadCtx[index].Handle) == -1) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " MainThreadWork :: Resume Scan thread failed! :: GLE= %d \n",GetLastError());
				printf("[-] Error :: MainThreadWork :: ResumeThread failed :: %d\n",GetLastError());
				ret = -6;

				__leave;
			}
		
			//Context->ScanThreadCtx[i] = threadCtx;		
			Context->Scan_thread_count++;
			i = Context->Scan_thread_count;
			printf("[+] Debug :: MainThreadWork :: Scan Thread [%d] created successfully! :: index = %d :: count = %d\n",Context->ScanThreadCtx[i-1].ThreadId,index, Context->Scan_thread_count);
			

		}

		

	}
	__finally {

		if (ret<0) {

			// Terminate all scan threads.			
			TerminateAllScanThreads(Context);			

			// Free allocated memory
			if (Context->ScanThreadCtx != NULL) {
				if (!(HeapFree(GetProcessHeap( ), 0, Context->ScanThreadCtx)) ) {
					printf("[-] Error :: MainThreadWork :: HeapFree failed with error :: %d\n",GetLastError());
				}
			}
			
			// Free allocated memory
			if (Context->MainThreadCtx != NULL) {
				if (!(HeapFree(GetProcessHeap( ), 0, Context->MainThreadCtx)) ) {
					printf("[-] Error :: MainThreadWork :: HeapFree failed with error :: %d\n",GetLastError());
				}
			}
			Context->MainThreadCtx = NULL;

		}
	}	

	return ret;
}

int Start_IHM_Connection(struct uhuru * uhuru, _Inout_ PONDEMAND_SCAN_CONTEXT Context) {

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

	if (uhuru == NULL) {
		printf("[-] Error :: Start_IHM_Connection :: NULL uhuru structure!\n");
		return -1;
	}

	Context->Uhuru = uhuru;

	__try {		

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
		Context->MainThreadCtx = mainThreadCtx;
		Context->Finalized = FALSE;
		Context->Scan_thread_count = 0;
		
		printf("[+] Debug :: start_IHM_Connection :: Main Thread [%d] created successfully!\n",mainThreadCtx->ThreadId);

		// Resuming the main thread.
		if (ResumeThread(Context->MainThreadCtx->Handle) == -1) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " Resuming main thread failed! :: GLE= %d \n",GetLastError());
			printf("[-] Error :: start_IHM_Connection :: ResumeThread failed :: %d\n",GetLastError());
			ret = -4;
			__leave;
		}


	}
	__finally {

		if (ret < 0) {

			if (Context->MainThreadCtx->Handle != NULL && Context->MainThreadCtx->Handle != INVALID_HANDLE_VALUE ) {
				CloseHandle(Context->MainThreadCtx->Handle);
				Context->ScanThreadCtx->Handle = NULL;
			}
			// Free allocated memory
			if (Context->MainThreadCtx != NULL) {
				if (!(HeapFree(GetProcessHeap( ), 0, Context->MainThreadCtx)) ) {
					printf("[-] Error :: start_IHM_Connection :: HeapFree failed with error :: %d\n",GetLastError());
				}
				Context->MainThreadCtx = NULL;
			}

		}

	}

	return ret;


}

int Close_IHM_Connection(_In_ PONDEMAND_SCAN_CONTEXT Context ) {

	int ret = 0;
	HANDLE hPipe = NULL;

	if (Context == NULL) {
		printf("[-] Error :: close_IHM_Connection :: NULL Context\n" );
		return -1;
	}

	__try {

		if (Context->PipeHandle != NULL && Context->PipeHandle != INVALID_HANDLE_VALUE ) {
			if (CloseHandle(Context->PipeHandle)) {
				printf("[+] Debug :: close_IHM_Connection :: Pipe closed successfully!\n");
				Context->PipeHandle = NULL;
			}
			else {
				printf("[-] Error :: close_IHM_Connection :: CloseHandle failed with error :: %d\n",GetLastError());
			}
			
		}

		// Terminate all scan threads
		TerminateAllScanThreads(Context);

		printf("[+] Debug :: close_IHM_Connection :: Terminating Main thread...\n");
		if (Context->MainThreadCtx != NULL && Context->MainThreadCtx->Handle != INVALID_HANDLE_VALUE) {

			// Terminate the main thread.
			if (!TerminateThread(Context->MainThreadCtx->Handle, 0)) {
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
		if (Context->MainThreadCtx != NULL) {
			if (!(HeapFree(GetProcessHeap( ), 0, Context->MainThreadCtx)) ) {
				printf("[-] Error :: start_IHM_Connection :: HeapFree failed with error :: %d\n",GetLastError());
			}
			Context->MainThreadCtx = NULL;
		}


	}
	__finally {

	}

	return ret;

}

int start_named_pipe_server(struct uhuru* uhuru)
{
	BOOL   fConnected = FALSE;
	DWORD  dwThreadId = 0;
	HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
	struct thread_parameters * params = NULL;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\uhuruAV_ondemand");

	// The main loop creates an instance of the named pipe and 
	// then waits for a client to connect to it. When the client 
	// connects, a thread is created to handle communications 
	// with that client, and this loop is free to wait for the
	// next client connect request. It is an infinite loop.

	for (;;)
	{
		_tprintf(TEXT("\nPipe Server: Main thread awaiting client connection on %s\n"), lpszPipename);
		hPipe = CreateNamedPipe(
			lpszPipename,             // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			BUFSIZE,                  // output buffer size 
			BUFSIZE,                  // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 

		if (hPipe == INVALID_HANDLE_VALUE)
		{
			_tprintf(TEXT("CreateNamedPipe failed, GLE=%d.\n"), GetLastError());
			return -1;
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 

		fConnected = ConnectNamedPipe(hPipe, NULL) ?
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (fConnected)
		{
			printf("Client connected, creating a processing thread.\n");
			params = (struct thread_parameters*)malloc(sizeof(struct thread_parameters));
			params->hPipe = hPipe;
			params->uhuru = uhuru;

			// Create a thread for this client. 
			hThread = CreateThread(
				NULL,              // no security attribute 
				0,                 // default stack size 
				InstanceThread,    // thread proc
				params,			   // thread parameter 
				0,                 // not suspended 
				&dwThreadId);      // returns thread ID 

			if (hThread == NULL)
			{
				free(params);
				_tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError());
				return -1;
			}
			else{
				CloseHandle(hThread);
			}
		}
		else
			// The client could not connect, so close the pipe. 
			CloseHandle(hPipe);
	}

	return 0;
}

int WINAPI InstanceThreadWork( PONDEMAND_SCAN_CONTEXT Context) {

	int ret = 0;

	HANDLE hHeap = GetProcessHeap();
	TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));
	TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));

	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	
	//HANDLE* hPipe = NULL;

	struct new_scan_action* scan; 

	// Do some extra error checking since the app will keep running even if this
	// thread fails.

	//struct thread_parameters * params = (struct thread_parameters*)lpvParam;
	//struct thread_parameters * params = (struct thread_parameters*)lpvParam;
	printf("[+] Debug :: In function InstanceThreadWork()\n");


	if (Context->PipeHandle == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL value in lpvParam.\n");
		printf("   InstanceThread exitting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		//if (params != NULL) free(params);
		return (DWORD)-1;
	}

	if (pchRequest == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   InstanceThread exitting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		//if (params != NULL) free(params);
		return (DWORD)-1;
	}

	if (pchReply == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   InstanceThread exitting.\n");
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		//if (params != NULL) free(params);
		return (DWORD)-1;
	}

	// Print verbose messages. In production code, this should be for debugging only.
	printf("InstanceThread created, receiving and processing messages.\n");


	// Loop until done reading
	while (1)
	{

		// Read client requests from the pipe. This simplistic code only allows messages
		// up to BUFSIZE characters in length.
		fSuccess = ReadFile(
			Context->PipeHandle,        // handle to pipe 
			pchRequest,    // buffer to receive data 
			BUFSIZE*sizeof(TCHAR), // size of buffer 
			&cbBytesRead, // number of bytes read 
			NULL);        // not overlapped I/O 

		if (!fSuccess || cbBytesRead == 0)
		{

			if (GetLastError() == ERROR_BROKEN_PIPE)
			{
				printf("[-] Error :: InstanceThread: client disconnected.\n", GetLastError());
			}
			else
			{
				printf("[-] Error :: InstanceThread ReadFile failed, GLE=%d.\n", GetLastError());				
			}
			break;
		}

		scan = (struct new_scan_action*)malloc(sizeof(struct new_scan_action));

		// To avoid random data on buffer after data read
		pchRequest[cbBytesRead] = '\0';		

		// Process the incoming message. Json parsing -> fill scan variable
		GetAnswerToRequest(pchRequest, pchReply, &cbReplyBytes, scan);

		// Write the reply to the pipe. 
		fSuccess = WriteFile(
			Context->PipeHandle,        // handle to pipe 
			pchReply,     // buffer to write from 
			cbReplyBytes, // number of bytes to write 
			&cbWritten,   // number of bytes written 
			NULL);        // not overlapped I/O 

		if (!fSuccess || cbReplyBytes != cbWritten)
		{
			_tprintf(TEXT("InstanceThread WriteFile failed, GLE=%d.\n"), GetLastError());
			free(scan);
			break;
		}

		// scan_action here

		// Step 4 -- Start Scan here if message already sent
		if (strcmp(scan->scan_action, "new_scan") == 0){
			start_new_scan(scan, Context->Uhuru);
		}
		else if (strcmp(scan->scan_action, "cancel") == 0){
			cancel_current_scan(scan, Context->Uhuru);
		}

		// Free inside struct ?
		if (scan){
			free(scan);
		}
		
	}

	// Flush the pipe to allow the client to read the pipe's contents 
	// before disconnecting. Then disconnect the pipe, and close the 
	// handle to this pipe instance. 

	FlushFileBuffers(Context->PipeHandle);
	DisconnectNamedPipe(Context->PipeHandle);
	CloseHandle(Context->PipeHandle);

	HeapFree(hHeap, 0, pchRequest);
	HeapFree(hHeap, 0, pchReply);
	//if (params != NULL) free(params);

	printf("[+] Debug :: InstanceThread exiting.\n");
	return 1;



	return ret;

}

DWORD WINAPI InstanceThread(LPVOID lpvParam)
// This routine is a thread processing function to read from and reply to a client
// via the open pipe connection passed from the main loop. Note this allows
// the main loop to continue executing, potentially creating more threads of
// of this procedure to run concurrently, depending on the number of incoming
// client connections.
{
	HANDLE hHeap = GetProcessHeap();
	TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));
	TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));

	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	//HANDLE* hPipe = NULL;

	struct new_scan_action* scan; 

	// Do some extra error checking since the app will keep running even if this
	// thread fails.

	struct thread_parameters * params = (struct thread_parameters*)lpvParam;

	if (params->hPipe == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL value in lpvParam.\n");
		printf("   InstanceThread exitting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		if (params != NULL) free(params);
		return (DWORD)-1;
	}

	if (pchRequest == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   InstanceThread exitting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		if (params != NULL) free(params);
		return (DWORD)-1;
	}

	if (pchReply == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   InstanceThread exitting.\n");
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		if (params != NULL) free(params);
		return (DWORD)-1;
	}

	// Print verbose messages. In production code, this should be for debugging only.
	printf("InstanceThread created, receiving and processing messages.\n");


	// Loop until done reading
	while (1)
	{

		// Read client requests from the pipe. This simplistic code only allows messages
		// up to BUFSIZE characters in length.
		fSuccess = ReadFile(
			params->hPipe,        // handle to pipe 
			pchRequest,    // buffer to receive data 
			BUFSIZE*sizeof(TCHAR), // size of buffer 
			&cbBytesRead, // number of bytes read 
			NULL);        // not overlapped I/O 

		if (!fSuccess || cbBytesRead == 0)
		{

			if (GetLastError() == ERROR_BROKEN_PIPE)
			{
				_tprintf(TEXT("InstanceThread: client disconnected.\n"), GetLastError());
			}
			else
			{
				_tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
			}
			break;
		}

		scan = (struct new_scan_action*)malloc(sizeof(struct new_scan_action));

		// To avoid random data on buffer after data read
		pchRequest[cbBytesRead] = '\0';

		// Process the incoming message. Json parsing -> fill scan variable
		GetAnswerToRequest(pchRequest, pchReply, &cbReplyBytes, scan);

		// Write the reply to the pipe. 
		fSuccess = WriteFile(
			params->hPipe,        // handle to pipe 
			pchReply,     // buffer to write from 
			cbReplyBytes, // number of bytes to write 
			&cbWritten,   // number of bytes written 
			NULL);        // not overlapped I/O 

		if (!fSuccess || cbReplyBytes != cbWritten)
		{
			_tprintf(TEXT("InstanceThread WriteFile failed, GLE=%d.\n"), GetLastError());
			free(scan);
			break;
		}

		// scan_action here

		// Step 4 -- Start Scan here if message already sent
		if (strcmp(scan->scan_action, "new_scan") == 0){
			start_new_scan(scan, params->uhuru);
		}
		else if (strcmp(scan->scan_action, "cancel") == 0){
			cancel_current_scan(scan, params->uhuru);
		}

		// Free inside struct ?
		if (scan){
			free(scan);
		}
		
	}

	// Flush the pipe to allow the client to read the pipe's contents 
	// before disconnecting. Then disconnect the pipe, and close the 
	// handle to this pipe instance. 

	FlushFileBuffers(params->hPipe);
	DisconnectNamedPipe(params->hPipe);
	CloseHandle(params->hPipe);

	HeapFree(hHeap, 0, pchRequest);
	HeapFree(hHeap, 0, pchReply);
	if (params != NULL) free(params);

	printf("InstanceThread exiting.\n");
	return 1;
}

VOID GetAnswerToRequest(LPTSTR pchRequest,
	LPTSTR pchReply,
	LPDWORD pchBytes, struct new_scan_action* scan)
	// This routine is a simple function to print the client request to the console
	// and populate the reply buffer with a default data string. This is where you
	// would put the actual client request processing code that runs in the context
	// of an instance thread. Keep in mind the main thread will continue to wait for
	// and receive other client connections while the instance thread is working.
{
	struct json_object * jobj  = NULL;
	const char* response = NULL;

	printf("Client Request String: %ls\n", pchRequest);
	// _tprintf(TEXT("Client Request String: -%s-\n"), pchRequest);

	//void json_parse_and_print(json_object * jobj); /* Forward declaration */
	//const char* json_parse_and_process(json_object * jobj, struct new_scan_action* scan);  /* Forward declaration */


	// We parse on-demand scan requests from IHM
	jobj = json_tokener_parse(pchRequest);
	//json_parse_and_print(jobj);

	// In this function, a scan demand is made to libuhuru_core
	response = json_parse_and_process(jobj, scan);

	// Check the outgoing message to make sure it's not too long for the buffer.
	if (FAILED(StringCchCopy(pchReply, BUFSIZE, response)))
	{
		*pchBytes = 0;
		pchReply[0] = 0;
		printf("StringCchCopy failed, no outgoing message.\n");
		return;
	}

	*pchBytes = (lstrlen(pchReply))*sizeof(TCHAR);

	//printf("BUFSIZE = %d -- Written = %d bytes\n", BUFSIZE, *pchBytes);
	return;
}