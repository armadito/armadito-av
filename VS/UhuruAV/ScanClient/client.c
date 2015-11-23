#include "client.h"

// global
struct uhuru * uhuru;


HRESULT UserScanWorker(_In_  PUSER_SCAN_CONTEXT Context )
/*

	Pseudo code::
	while(TRUE) {
        1) Get a overlap structure from the completion port.
        2) Obtain message from overlap structure.
        3) Process the message via calling UserScanHandleStartScanMsg(...)
        4) Pump overlap structure into completion port using FilterGetMessage(...)
    }
*/
{

	HRESULT hres = S_OK;
	PSCANNER_THREAD_CONTEXT threadCtx = NULL;
	DWORD ThreadId =0;
	int i = 0;
	PSCANNER_MESSAGE message = NULL;
	SCANNER_REPLY_MESSAGE reply;
	//PSCANNER_MESSAGE msg = NULL;
	BOOL success = FALSE;
	DWORD outSize;
    ULONG_PTR key;
	LPOVERLAPPED pOvlp = NULL;
	char * msDosFilename = NULL;
	struct uhuru_report report;
	enum uhuru_file_status scan_result = UHURU_IERROR;


	// Get thread context by ID.
	ThreadId = GetCurrentThreadId( );

	//printf("\n[i] In thread worker routine...:: Thread ID : %d :: Context :: %d :: Finalize = %d\n",ThreadId, Context, Context->Finalized);
	printf("\n[i] In thread worker routine...:: Thread ID : %d :: Finalize = %d\n",ThreadId, Context->Finalized);
	for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {

		if (ThreadId == Context->ScanThreadCtxes[i].ThreadId ) {
			threadCtx = &Context->ScanThreadCtxes[i];
			break;
		}
	}

	if (threadCtx == NULL) {
		printf("[-] Error :: UserScanWorker :: Thread Not found\n");
		return S_FALSE;
	}	

	//ZeroMemory( &reply, SCANNER_REPLY_MESSAGE_SIZE );

	// This thread is waiting for scan order from the Driver.
	while (TRUE) {
		
		
		hres = S_OK;
		

		/*if (threadCtx->Aborted == 1) { 
			printf("\n[i] Debug  :: UserScanWorker ::Aborted flag set in the main thread\n");
            break;
        }*/

		message = NULL;

		//
        //  Get overlapped structure asynchronously, the overlapped structure 
        //  was previously pumped by FilterGetMessage(...)
        //
        
        success = GetQueuedCompletionStatus( Context->Completion, &outSize, &key, &pOvlp, INFINITE );

		if (!success) {
        
            hres = HRESULT_FROM_WIN32(GetLastError());
            
            //
            //  The completion port handle associated with it is closed 
            //  while the call is outstanding, the function returns FALSE, 
            //  *lpOverlapped will be NULL, and GetLastError will return ERROR_ABANDONED_WAIT_0
            //
            
            if (hres == E_HANDLE) {
            
                printf("[-] Error :: UserScanWorker :: Completion port becomes unavailable.\n");
                hres = S_OK;
                
            } else if (hres == HRESULT_FROM_WIN32(ERROR_ABANDONED_WAIT_0)) {
            
				printf("[-] Error :: UserScanWorker :: Completion port was closed.\n");
                hres = S_OK;
            }

            break;
        }


		//
        //  Recover message strcuture from overlapped structure.
        //  Remember we embedded overlapped structure inside SCANNER_MESSAGE.
        //  This is because the overlapped structure obtained from GetQueuedCompletionStatus(...)
        //  is asynchronously and not guranteed in order. 
        //
        
        message = CONTAINING_RECORD( pOvlp, SCANNER_MESSAGE, Ovlp );

		if (message == NULL) {
			printf("[-] Warning :: UserScanWorker :: [%d] :: Get message from driver failed :: hres = 0x%x.\n",ThreadId, hres);
		}
		/*
		else {
			printf("[+] Debug :: UserScanWorker :: Thread %d :: Message received successfully !!.\n",ThreadId);
		}*/
        
        if (message->msg.FileName != NULL) {

			
			//printf("\n[+] Debug :: UserScanWorker :: Thread %d Scan order received for file :: %s ::\n",ThreadId, message->msg.FileName);

			// init reply struct.
			//ZeroMemory( &reply, SCANNER_REPLY_MESSAGE_SIZE );
			//ZeroMemory(&reply->ReplyHeader, sizeof(FILTER_REPLY_HEADER));
			//reply->ReplyHeader.Status = 0;


			// Get the MS-DOS filename 
			msDosFilename = ConvertDeviceNameToMsDosName(message->msg.FileName);
			//printf("\n[+] Debug :: UserScanWorker :: Thread %d :: msDosFilename = %s ::\n",ThreadId,msDosFilename);

			// fake scan example
			if (strstr(msDosFilename,"UH_MALWARE") != NULL) {
				scan_result = UHURU_MALWARE;
			}
			else {

				// Initialize the scan report structure
				uhuru_report_init(&report, 1, msDosFilename, 0);

				// launch a simple file scan
				scan_result = uhuru_scan_simple(uhuru, msDosFilename, &report);

			}

			ZeroMemory( &reply, SCANNER_REPLY_MESSAGE_SIZE);
			reply.ReplyHeader.MessageId = message->MessageHeader.MessageId;

			reply.scanResult = scan_result;


			//reply.scanResult = DBG_FLAG;			

			

			
			//printf("[DEBUG].... %d before FilterReplyMessage....\n",ThreadId);
			// Send a reply message to the driver.
			hres = FilterReplyMessage(Context->ConnectionPort,&reply.ReplyHeader,SCANNER_REPLY_MESSAGE_SIZE);
			//printf("[DEBUG].... %d after FilterReplyMessage....\n",ThreadId);

			if(hres != S_OK){
				printf("[-] Error :: UserScanWorker :: Thread %d :: FilterReplyMessage failed :: hres = 0x%x.\n",ThreadId, hres);
				
			}
			else {
				//printf("[+] Debug :: UserScanWorker :: [%d  :: Reply sent successfully to the driver\n",ThreadId);
				switch (scan_result) {

					case 0:
						printf("[+] Debug :: UserScanWorker :: [%d] :: %s :: NONE\n",ThreadId,msDosFilename);
						break;
					case UHURU_UNDECIDED:
						printf("[+] Debug :: UserScanWorker :: [%d] :: %s :: UHURU_UNDECIDED\n",ThreadId,msDosFilename);					
						break;
					case UHURU_CLEAN:
						printf("[+] Debug :: UserScanWorker :: [%d] :: %s :: UHURU_CLEAN\n",ThreadId,msDosFilename);
						break;
					case UHURU_UNKNOWN_FILE_TYPE:
						printf("[+] Debug :: UserScanWorker :: [%d] :: %s :: UHURU_UNKNOWN_FILE_TYPE\n",ThreadId,msDosFilename);
						break;
					case UHURU_EINVAL:
						printf("[+] Debug :: UserScanWorker :: [%d] :: %s :: UHURU_EINVAL\n",ThreadId,msDosFilename);
						break;
					case UHURU_IERROR:
						printf("[+] Debug :: UserScanWorker :: [%d] :: %s :: UHURU_IERROR\n",ThreadId,msDosFilename);
						break;
					case UHURU_SUSPICIOUS:
						printf("[+] Debug :: UserScanWorker :: [%d] :: %s :: UHURU_SUSPICIOUS\n",ThreadId,msDosFilename);
						break;
					case UHURU_WHITE_LISTED:
						printf("[+] Debug :: UserScanWorker :: [%d] :: %s :: UHURU_WHITE_LISTED\n",ThreadId,msDosFilename);
						break;
					case UHURU_MALWARE:
						printf("[+] Debug :: UserScanWorker :: [%d] :: %s :: UHURU_MALWARE\n",ThreadId,msDosFilename);
						break;
					default:
						printf("[+] Debug :: UserScanWorker :: [%d] :: %s :: ERROR_DEFAULT \n",ThreadId);
						break;

				}
			}



#if 0
            //
            //  Reset the abort flag since this is a new scan request and remember 
            //  the scan context ID. This ID will allow us to match a cancel request 
            //  with a given scan task.
            //

            EnterCriticalSection(&(threadCtx->Lock));
            threadCtx->Aborted = FALSE;
            threadCtx->ScanId = message->Notification.ScanId;
            LeaveCriticalSection(&(threadCtx->Lock));
        
            //
            //  Reply the scanning worker thread handle to the filter
            //  This is important because the filter will also wait for the scanning thread 
            //  in case that the scanning thread is killed before telling filter 
            //  the scan is done or aborted.
            //
            
            ZeroMemory( &replyMsg, SCANNER_REPLY_MESSAGE_SIZE );
            replyMsg.ReplyHeader.MessageId = message->MessageHeader.MessageId;
            replyMsg.ThreadId = threadCtx->ThreadId;
            hr = FilterReplyMessage( Context->ConnectionPort,
                                     &replyMsg.ReplyHeader,
                                     SCANNER_REPLY_MESSAGE_SIZE );
    
            if (FAILED(hr)) {

                fprintf(stderr,
                  "[UserScanWorker]: Failed to reply thread handle to the minifilter\n");
                DisplayError(hr);
                break;
            }
            hr = UserScanHandleStartScanMsg( Context, message, threadCtx );

#endif
                
        }
        
		/*
        if (FAILED(hres)) {           
			printf("[-] Warning :: UserScanWorker :: Thread %d :: FilterGetMessage failed :: hres = 0x%x.\n",ThreadId, hres);
        }*/


#if 0
		// pump the 
		hres = FilterGetMessage(Context->ConnectionPort,&msg->MessageHeader,sizeof(SCANNER_MESSAGE),NULL);
		if (hres != S_OK) {
			printf("[-] Warning :: UserScanWorker :: Thread %d :: FilterGetMessage failed :: hres = 0x%x.\n",ThreadId, hres);
		}
		else {
			printf("[+] Debug :: UserScanWorker :: Thread %d :: Message received successfully !!.\n");
		}

#endif

		// If the finalized flag is set from the main thread.		
		if (Context->Finalized == 1) { 
			printf("\n[i] Debug  :: UserScanWorker :: Finalized flag set in the main thread !\n");
            break;
        }
		
		//
        //  After we process the message, pump a overlapped structure into completion port again.
        //
        
        hres = FilterGetMessage( Context->ConnectionPort, &message->MessageHeader, FIELD_OFFSET(SCANNER_MESSAGE, Ovlp ), &message->Ovlp );

        if (hres == HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED)) {
            
            printf("[-] Warning :: UserScanWorker :: FilterGetMessage aborted.\n");
            break;
            
        } else if (hres != HRESULT_FROM_WIN32( ERROR_IO_PENDING )) {
			printf("[-] Warning :: UserScanWorker :: Thread %d :: FilterGetMessage failed :: hres = 0x%x.\n",ThreadId, hres);                       
            break;
        }
		



	} // End of while


	if (message != NULL) {

        //
        //  Free the memory, which originally allocated at UserScanInit(...)
        //
        
        HeapFree(GetProcessHeap(), 0, message);
    }
	printf("\n[i] Debug  :: UserScanWorker :: Thread id %d exiting\n",ThreadId);

	return hres;
}


HRESULT UserScanInit(_Inout_  PUSER_SCAN_CONTEXT Context) {

	int i = 0;
	HRESULT hRes = S_OK;
	PSCANNER_THREAD_CONTEXT  scanThreadCtxes = NULL;
	UCHAR ServiceIdentificationPassword[] = {0xBA, 0xDA, 0x5E, 0x52, 0x56, 0x1C, 0xE0};

	// Check parameter.
	if (Context == NULL) {
		printf("[-] Error :: UserScanInit :: NULL parameter Context\n" );
		hRes = E_INVALIDARG;
		return hRes;
	}


	__try {



		// Initialize scan thread contexts. (containing threadID, handle to the thread, 
		scanThreadCtxes = HeapAlloc(GetProcessHeap(), 0, sizeof(SCANNER_THREAD_CONTEXT) * USER_SCAN_THREAD_COUNT);
		if (NULL == scanThreadCtxes) {    
			hRes = MAKE_HRESULT(SEVERITY_ERROR, 0, E_OUTOFMEMORY);
			__leave;
		}

		ZeroMemory(scanThreadCtxes, sizeof(SCANNER_THREAD_CONTEXT) * USER_SCAN_THREAD_COUNT);


		// Create Scan Listening threads.
		for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {

			// 
			scanThreadCtxes[i].Handle = CreateThread( NULL,	// Thread Security Attribute
										0,		// Stack Size
										(LPTHREAD_START_ROUTINE)UserScanWorker,	// Function to be executed by the thread.
										Context,			// Paramater passed to the function.
										CREATE_SUSPENDED,		// Creation flag. // Create_SUSPENDED = The thread is created in a suspend state.
										&scanThreadCtxes[i].ThreadId );


			if (scanThreadCtxes[i].Handle == NULL) {				
				hRes = HRESULT_FROM_WIN32(GetLastError());
				printf("[-] Error :: CreateThread() failed :: errcode = 0x%x\n",hRes);
				__leave;		
			}
			InitializeCriticalSection(&(scanThreadCtxes[i].Lock));
		}

		//printf("[+] Debug :: UserScanInit :: Scan thread created successfully !!\n");

#if 1
		// Try to connect to the driver communication port.
		hRes = FilterConnectCommunicationPort(SCAN_PORT_NAME,	// port name
										 0,			// Connection Options
										 &ServiceIdentificationPassword,				// Context information 
										 RTL_NUMBER_OF(ServiceIdentificationPassword),	// size of context
										 NULL,		// Security Attributes.
										 &Context->ConnectionPort);

		if (FAILED(hRes)) {
			printf("[-] Error :: FilterConnectCommunicationPort() failed :: errcode = 0x%x\n",hRes);
			__leave;
		}
		printf("[+] Debug :: UserScanInit :: Client connected to the driver communication port !!\n");

		
		//  Create the IO completion port for asynchronous message passing. // Why ?? // try to remove this line to see cons
		Context->Completion = CreateIoCompletionPort( Context->ConnectionPort,
													 NULL,
													 0,
													 USER_SCAN_THREAD_COUNT );

		if ( NULL == Context->Completion ) {
			hRes = HRESULT_FROM_WIN32(GetLastError());
			__leave;
		}
#endif
		




		Context->Finalized = FALSE;
		Context->ScanThreadCtxes = scanThreadCtxes;
		Context->AbortThreadHandle = NULL; // don't need this parameter for the moment.
		//
		// Resume all the scanning threads.
		for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {

			if ( ResumeThread( scanThreadCtxes[i].Handle ) == -1) {         							
				hRes = HRESULT_FROM_WIN32(GetLastError());
				printf("[-] Error :: UserScanInit :: ResumeThread() failed on thread No%d:: errcode = 0x%x\n",i,hRes);
				__leave;
			 }

		}


		//  Pump messages into queue of completion port.
		for (i = 0; i < USER_SCAN_THREAD_COUNT; i ++ ) {

			PSCANNER_MESSAGE msg = HeapAlloc( GetProcessHeap(), 0, sizeof( SCANNER_MESSAGE ) );
        
			if (NULL == msg) {
        
				hRes = MAKE_HRESULT(SEVERITY_ERROR, 0, E_OUTOFMEMORY);
				__leave;
			}
        
			FillMemory( &msg->Ovlp, sizeof(OVERLAPPED), 0);
			hRes = FilterGetMessage( Context->ConnectionPort,  &msg->MessageHeader,  FIELD_OFFSET( SCANNER_MESSAGE, Ovlp ),  &msg->Ovlp );

			if (hRes == HRESULT_FROM_WIN32( ERROR_IO_PENDING )) {
        
				hRes = S_OK;
            
			} else {
        
				printf("[-] Error :: UhuruSvc!UserScanInit :: FilterGetMessage failed :: GLE=%03d. \n",GetLastError());			
				HeapFree(GetProcessHeap(), 0, msg );
				__leave;
			}
		}


	}
	__finally{

		// In case of failure.
		if (hRes != S_OK) {

			//printf("[DEBUG] :: finally section...\n");

			// Close completion handle.
			if (Context->Completion && CloseHandle(Context->Completion) == FALSE) {    			
				printf("[-] Error :: UhuruSvc!UserScanInit :: CloseHandle for completion port failed :: GLE=%03d. \n",GetLastError());			
			}

			// Close communication port.
			if ((Context->ConnectionPort != INVALID_HANDLE_VALUE && Context->ConnectionPort != NULL ) && !CloseHandle(Context->ConnectionPort)) {
				printf("[-] Error :: UhuruSvc!UserScanInit :: CloseHandle for communication port has failed :: GLE=%03d. \n",GetLastError());			
			}
			else {
				printf("[+] Debug :: UhuruSvc!UserScanInit :: Communication port closed Successfully \n");			
			}


			// Close all threads handles
			if (scanThreadCtxes != NULL) {

				for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {

					if (scanThreadCtxes[i].Handle && CloseHandle(scanThreadCtxes[i].Handle) == FALSE ) {
						printf("[-] Error :: UhuruSvc!UserScanInit :: CloseHandle failed for thread %d :: GLE=%03d. \n",i,GetLastError());
						DeleteCriticalSection(&(scanThreadCtxes[i].Lock));
					}

				}

				// Free the allocated contexts.
				HeapFree(GetProcessHeap(), 0, scanThreadCtxes);

			}


		}
		


	}


	return hRes;
}


HRESULT UserScanFinalize(_In_  PUSER_SCAN_CONTEXT Context) {

	HRESULT hres = S_OK;
	HANDLE hScanThreads[USER_SCAN_THREAD_COUNT] = {0};
	int i = 0;
	
	if (Context->ScanThreadCtxes == NULL) {
		printf("[-] Error :: UserScanFinalize :: NULL Scan Thread Contexts\n");
		//hres = E_POINTER;
		return hres;
	}

	printf("[+] Debug :: UserScanFinalize :: Finalizing threads....\n");

	// Tell all scannig threads that the program is going to exit.	
	Context->Finalized = TRUE;
	
	printf("[DEBUG1]...\n");
	for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {
		printf("Thread id : %d\n", Context->ScanThreadCtxes[i].ThreadId);		
		Context->ScanThreadCtxes[i].Aborted = TRUE;
			 
	}
	//printf("[DEBUG2]...\n");

	// Wake up the listeing thread if it is waiting for messag via GetQueuedCompletionStatus()
	CancelIoEx(Context->ConnectionPort, NULL);

	//printf("[DEBUG3]...\n");
	
	// Wait for all scan threads to complete cancellation, so we will able to close the connection port.
	for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {
		if (Context->ScanThreadCtxes[i].Handle == INVALID_HANDLE_VALUE || Context->ScanThreadCtxes[i].Handle == NULL ) {
			printf("[Error] :: UserScanFinalize :: NULL Thread Handle\n");
		}
		hScanThreads[i] = Context->ScanThreadCtxes[i].Handle;
	}
	printf("[DEBUG4]...\n");

	WaitForMultipleObjects(USER_SCAN_THREAD_COUNT,hScanThreads,TRUE,INFINITE );

	printf("[DEBUG5]...\n");
		
	

	printf("[+] Debug :: UserScanFinalize :: Closing connection port.\n");


	// close all scan ports.

	if ( (Context->ConnectionPort != INVALID_HANDLE_VALUE && Context->ConnectionPort != NULL ) && !CloseHandle(Context->ConnectionPort)) {
		printf("[-] Error :: UserScanFinalize :: Closehandle connection port failed !\n");
		hres = HRESULT_FROM_WIN32(GetLastError());		
	}
	Context->ConnectionPort = NULL;

	if (!CloseHandle(Context->Completion)) {
		printf("[-] Error :: UserScanFinalize :: CloseHandle completion port failed !\n");
		hres = HRESULT_FROM_WIN32(GetLastError());
	}

	Context->Completion = NULL;

	//  Clean up scan thread contexts
	for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {

		if (Context->ScanThreadCtxes[i].Handle && CloseHandle(Context->ScanThreadCtxes[i].Handle) == FALSE ) {
			printf("[-] Error :: UhuruSvc!UserScanFinalize :: CloseHandle failed for thread %d :: GLE=%03d. \n",i,GetLastError());
			DeleteCriticalSection(&(Context->ScanThreadCtxes[i].Lock));
		}

	}

	// Fre the allocated contexts.
	HeapFree(GetProcessHeap(), 0, Context->ScanThreadCtxes);
	Context->ScanThreadCtxes = NULL;



	return hres; 
}


char * ConvertDeviceNameToMsDosName(LPSTR DeviceFileName) {
	
	char deviceDosName[BUFSIZE];
	char deviceLetter[3] = {'\0'};
	char deviceNameQuery[BUFSIZE] = {'\0'};
	char * deviceDosFilename = NULL;
	DWORD len = 0;
	DWORD len2 = 0;
	DWORD ret = 0;
	BOOL bFound = FALSE;
	//char * p;
	char * tmp;
	//char * tmp2;
	

	if (DeviceFileName == NULL) {
		printf("[-] Error :: ConvertDeviceNameToMsDosName :: null parameter DeviceName\n");
		return NULL;
	}

	// Get the list of the logical drives.
	len = GetLogicalDriveStringsA(BUFSIZE,deviceDosName);
	if (len == 0) {
		printf("[-] Error :: ConvertDeviceNameToMsDosName!GetLogicalDriveStrings() failed ::  error code = 0x%03d\n",GetLastError());
		return NULL;
	}
		
	
	tmp = deviceDosName;

	do {

		//printf("[+] Debug :: deviceDosName = %s\n",tmp);

		// Get the device letter without the backslash (Ex: C:).
		memcpy_s(deviceLetter,2,tmp,2);
		
		if (!QueryDosDeviceA(deviceLetter, deviceNameQuery, BUFSIZE)) {
			printf("[-] Error :: QueryDosDeviceA() failed ::  error code = 0x%03d\n",GetLastError());
			return NULL;
		}
		//printf("[+] Debug :: DeviceName = %s ==> %s\n",deviceNameQuery,deviceLetter);

		if (strstr(DeviceFileName,deviceNameQuery) != NULL) {
			//printf("[+] Debug :: FOUND DeviceName = %s ==> %s\n",deviceNameQuery,deviceLetter);

			len2 = strnlen_s(deviceNameQuery, MAX_PATH_SIZE);
			len = strnlen_s(DeviceFileName, MAX_PATH_SIZE) - len2 + 3;					
			
			deviceDosFilename = (char*)calloc(len+1,sizeof(char) );
			deviceDosFilename[len] = '\0';

			memcpy_s(deviceDosFilename, len, tmp, 3);
			memcpy_s(deviceDosFilename+2, len, DeviceFileName+len2 , len-1);
			//printf("[+] DEBUG :: deviceDosFilename = %s\n",deviceDosFilename);
			
			bFound = TRUE;

		}

		// got to the next device name.
		while (*tmp++);
		//printf("[+] Debug :: next device name = %s\n",tmp);

		//found = TRUE;
		
		
	} while (bFound == FALSE && *tmp);


	if (bFound == FALSE) {
		return NULL;
	}
	

	return deviceDosFilename;
}


int main(int argc, char ** argv) {

	HRESULT hres = S_OK;			
	USER_SCAN_CONTEXT userScanCtx = {0};
	UCHAR c;
	uhuru_error *err = NULL;
	
	uhuru = uhuru_open(&err);
	if (uhuru == NULL){
		printf("[-] Error :: uhuru_open() error - %s \n", err->error_message );
		return -1;
	}
	printf("[+] Debug :: uhuru struct initialized successfully!\n");

	

	//  Initialize scan listening threads.
	hres = UserScanInit(&userScanCtx);
	
	while(1) {
		 printf("press 'q' to quit: ");
        c = (unsigned char) getchar();
        if (c == 'q') {
        
            break;
        }
	}	

	//printf("\n[i] Context in main thread :: %d\n", &userScanCtx);

	// Finalize the scan thread contexts.
	hres = UserScanFinalize(&userScanCtx);

	err = NULL;
	uhuru_close(uhuru,&err);
	printf("[+] Debug :: uhuru struct closed successfully!\n");

	return EXIT_SUCCESS;
}


#if 0
int main(int argc, char * argv) {

	uhuru_error *err = NULL;
	struct uhuru * uhuru;
	struct uhuru_report report;
	enum uhuru_file_status scan_result = UHURU_IERROR;
	//const char * filepath = "C:\\Users\\david\\Desktop\\to_analyze\\note.txt";
	const char * filepath = "D:\\Reverse\\Malware\\peMalware\\10009b9e47ddd691ccc9ae04cd9cc0bb";

	uhuru = uhuru_open(&err);
	if (uhuru == NULL){
		printf("uhuru_open() error - %s \n", err->error_message );
		return -1;
	}

	printf("[+] Debug :: uhuru struct initialized successfully!\n");
	//printf("uhuru-report %s\n",report.mod_report);
	uhuru_report_init(&report, 1, filepath, 0);

	// launch a simple file scan
	scan_result = uhuru_scan_simple(uhuru,filepath,&report);

	printf("[i] Debug :: modname = %s :: modreport = %s\n",report.mod_name,report);

	switch (scan_result) {

		case UHURU_UNDECIDED:
			printf("[+] Debug :: Scan_result :: UHURU_UNDECIDED\n");
			break;
		case UHURU_CLEAN:
			printf("[+] Debug :: Scan_result :: UHURU_CLEAN\n");
			break;
		case UHURU_UNKNOWN_FILE_TYPE:
			printf("[+] Debug :: Scan_result :: UHURU_UNKNOWN_FILE_TYPE\n");
			break;
		case UHURU_EINVAL:
			printf("[+] Debug :: Scan_result :: UHURU_EINVAL\n");
			break;
		case UHURU_IERROR:
			printf("[+] Debug :: Scan_result :: UHURU_IERROR\n");
			break;
		case UHURU_SUSPICIOUS:
			printf("[+] Debug :: Scan_result :: UHURU_SUSPICIOUS\n");
			break;
		case UHURU_WHITE_LISTED:
			printf("[+] Debug :: Scan_result :: UHURU_WHITE_LISTED\n");
			break;
		case UHURU_MALWARE:
			printf("[+] Debug :: Scan_result :: UHURU_MALWARE\n");
			break;
		default:
			printf("[+] Debug :: Scan_result :: ERROR_DEFAULT \n");
			break;

	}
	


	err = NULL;
	uhuru_close(uhuru,&err);

	//system("pause");

	return 0;
}
#endif