#include "scan_on_access.h"
#include "log.h"

extern struct uhuru * uhuru;

const CHAR * PrintUhuruScanResult(SCAN_RESULT res) {

	switch (res) {

			case 0:
				return "[NONE]";				
				break;
			case UHURU_UNDECIDED:
				return "[UHURU_UNDECIDED]";				
				break;
			case UHURU_CLEAN:
				return "[UHURU_CLEAN]";
				break;
			case UHURU_UNKNOWN_FILE_TYPE:
				return "[UHURU_UNKNOWN_FILE_TYPE]";
				break;
			case UHURU_EINVAL:
				return "[UHURU_EINVAL]";
				break;
			case UHURU_IERROR:
				return "[UHURU_IERROR]";
				break;
			case UHURU_SUSPICIOUS:
				return "[UHURU_SUSPICIOUS]";
				break;
			case UHURU_WHITE_LISTED:
				return "[UHURU_WHITE_LISTED]";
				break;
			case UHURU_MALWARE:
				return "[UHURU_MALWARE]";
				break;			
			default:
				return "[ERROR_DEFAULT]";
				break;
		}
	
}

HRESULT UserScanWorker( _In_  PUSER_SCAN_CONTEXT Context )
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
	
	uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_DEBUG, " [i] In thread worker routine...:: Thread ID : %d :: Finalize = %d\n",ThreadId, Context->Finalized);
	uhLog("\n[i] In thread worker routine...:: Thread ID : %d :: Finalize = %d\n",ThreadId, Context->Finalized);
	
	for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {

		if (ThreadId == Context->ScanThreadCtxes[i].ThreadId ) {
			threadCtx = &Context->ScanThreadCtxes[i];
			break;
		}
	}

	if (threadCtx == NULL) {
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " UhuruSvc!UserScanWorker :: NULL Thread context\n");
		//uhLog("[-] Error :: UserScanWorker :: Thread Not found\n");
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
            
                //uhLog("[-] Error :: UserScanWorker :: Completion port becomes unavailable.\n");
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " UhuruSvc!UserScanWorker :: Completion port becomes unavailable.\n");
                hres = S_OK;
                
            } else if (hres == HRESULT_FROM_WIN32(ERROR_ABANDONED_WAIT_0)) {
				
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " UhuruSvc!UserScanWorker :: Completion port closed unexpectedly.\n");
				uhLog("[-] Error :: UserScanWorker :: Completion port was closed.\n");
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
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_WARNING, " UhuruSvc!UserScanWorker :: [%d] :: Get message from driver failed :: hres = 0x%x.\n",ThreadId, hres);
			//uhLog("[-] Warning :: UserScanWorker :: [%d] :: Get message from driver failed :: hres = 0x%x.\n",ThreadId, hres);
		}		
        
        if (message != NULL && message->msg.FileName != NULL) {			

			// Get the MS-DOS filename 
			msDosFilename = ConvertDeviceNameToMsDosName(message->msg.FileName);


			if (msDosFilename == NULL) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_WARNING, " UhuruSvc!UserScanWorker :: [%d] :: ConvertDeviceNameToMsDosName failed :: \n",ThreadId);
				uhLog("[-] Error :: UserScanWorker :: [%d] :: ConvertDeviceNameToMsDosName failed :: \n",ThreadId);
				scan_result = UHURU_EINVAL;
			}
			// fake scan example			
			else if (strstr(msDosFilename,"UH_MALWARE") != NULL) {				
				scan_result = UHURU_MALWARE;
			}
			else if (strstr(msDosFilename,"UHURU.TXT") != NULL) {  // Do not scan the log file.
				scan_result = UHURU_WHITE_LISTED;
			}
			else {

				// launch a simple file scan
				scan_result = uhuru_scan_simple(uhuru, msDosFilename, &report);

			}

			ZeroMemory( &reply, SCANNER_REPLY_MESSAGE_SIZE);
			reply.ReplyHeader.MessageId = message->MessageHeader.MessageId;

			reply.scanResult = scan_result;

			// Send a reply message to the driver.
			hres = FilterReplyMessage(Context->ConnectionPort,&reply.ReplyHeader,SCANNER_REPLY_MESSAGE_SIZE);			

			if(FAILED(hres)){
				//uhLog("[-] Error :: UserScanWorker :: Thread %d :: FilterReplyMessage failed :: hres = 0x%x.\n",ThreadId, hres);
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_WARNING, " UhuruSvc!UserScanWorker :: [%d] :: FilterReplyMessage failed :: hres = 0x%x.\n",ThreadId, hres);
			}
			else {
				if (scan_result == UHURU_MALWARE)
					uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_WARNING, " UhuruSvc!UserScanWorker :: Malware found !! \n file: [%s] ",msDosFilename);
				
				uhLog("[+] Debug :: UserScanWorker :: [%d] :: %s :: %s\n",ThreadId,msDosFilename,PrintUhuruScanResult(scan_result));
			}


			if (msDosFilename != NULL) {
				free(msDosFilename);
				msDosFilename = NULL;
			}

                
        }
        		

		// If the finalized flag is set from the main thread.		
		if (Context->Finalized == 1) {			
			uhLog("\n[i] Debug  :: UserScanWorker :: Finalized flag set in the main thread !\n");
            break;
        }
		
		//
        //  After we process the message, pump a overlapped structure into completion port again.
        //
        
        hres = FilterGetMessage( Context->ConnectionPort, &message->MessageHeader, FIELD_OFFSET(SCANNER_MESSAGE, Ovlp ), &message->Ovlp );

        if (hres == HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED)) {
            			
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " UhuruSvc!UserScanWorker :: FilterGetMessage aborted unexpectedly");
            uhLog("[-] Warning :: UserScanWorker :: FilterGetMessage aborted.\n");
            break;
            
        } else if (hres != HRESULT_FROM_WIN32( ERROR_IO_PENDING )) {
			
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_WARNING, " UhuruSvc!UserScanWorker :: [%d] :: FilterGetMessage failed :: hres = 0x%x.\n",ThreadId, hres);
			uhLog("[-] Warning :: UserScanWorker :: Thread %d :: FilterGetMessage failed :: hres = 0x%x.\n",ThreadId, hres);                       
            break;
        }
		



	} // End of while


	if (msDosFilename != NULL) {
		free(msDosFilename);
		msDosFilename = NULL;
	}

	if (message != NULL) {

        //
        //  Free the memory, which originally allocated at UserScanInit(...)
        //        
        HeapFree(GetProcessHeap(), 0, message);
    }

	uhLog("\n[i] Debug  :: UserScanWorker :: Thread id %d exiting\n",ThreadId);

	return hres;
}

HRESULT UserScanInit(_Inout_  PUSER_SCAN_CONTEXT Context) {

	int i = 0;
	HRESULT hRes = S_OK;
	PSCANNER_THREAD_CONTEXT  scanThreadCtxes = NULL;
	UCHAR ServiceIdentificationPassword[] = {0xBA, 0xDA, 0x5E, 0x52, 0x56, 0x1C, 0xE0};

	// Check parameter.
	if (Context == NULL) {
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " UserScanInit :: NULL context parameter\n");
		//uhLog("[-] Error :: UserScanInit :: NULL parameter Context\n" );
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
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " UserScanInit :: CreateThread() failed :: errcode = 0x%x\n",hRes);
				//uhLog("[-] Error :: CreateThread() failed :: errcode = 0x%x\n",hRes);
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
			uhLog("[-] Error :: FilterConnectCommunicationPort() failed :: errcode = 0x%x\n",hRes);
			__leave;
		}
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_INFO, " Service connected to the driver communication port\n");
		//uhLog("[+] Debug :: UserScanInit :: Client connected to the driver communication port !!\n");

		
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
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " UserScanInit :: ResumeThread() failed on thread [%d] :: errcode = 0x%x\n",i,hRes);
				//uhLog("[-] Error :: UserScanInit :: ResumeThread() failed on thread [%d] :: errcode = 0x%x\n",i,hRes);
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
        
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, " UserScanInit :: FilterGetMessage failed :: GLE=%03d. \n",GetLastError());
				//uhLog("[-] Error :: UhuruSvc!UserScanInit :: FilterGetMessage failed :: GLE=%03d. \n",GetLastError());			
				HeapFree(GetProcessHeap(), 0, msg );
				__leave;
			}
		}


	}
	__finally{

		// In case of failure.
		if (hRes != S_OK) {			

			// Close completion handle.
			if (Context->Completion && CloseHandle(Context->Completion) == FALSE) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, "  UhuruSvc!UserScanInit :: CloseHandle for completion port failed :: GLE=%03d. \n",GetLastError());
				//uhLog("[-] Error :: UhuruSvc!UserScanInit :: CloseHandle for completion port failed :: GLE=%03d. \n",GetLastError());			
			}

			// Close communication port.
			if ((Context->ConnectionPort != INVALID_HANDLE_VALUE && Context->ConnectionPort != NULL ) && !CloseHandle(Context->ConnectionPort)) {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, "  UhuruSvc!UserScanInit :: CloseHandle for communication port has failed :: GLE=%03d. \n",GetLastError());
				//uhLog("[-] Error :: UhuruSvc!UserScanInit :: CloseHandle for communication port has failed :: GLE=%03d. \n",GetLastError());			
			}
			else {
				uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_INFO, "  UhuruSvc!UserScanInit :: Communication port closed Successfully");
				//uhLog("[+] Debug :: UhuruSvc!UserScanInit :: Communication port closed Successfully \n");			
			}


			// Close all threads handles
			if (scanThreadCtxes != NULL) {

				for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {

					if (scanThreadCtxes[i].Handle && CloseHandle(scanThreadCtxes[i].Handle) == FALSE ) {
						uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR, "  UhuruSvc!UserScanInit :: CloseHandle failed for thread %d :: GLE=%03d. \n",i,GetLastError());
						//uhLog("[-] Error :: UhuruSvc!UserScanInit :: CloseHandle failed for thread %d :: GLE=%03d. \n",i,GetLastError());
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

int initializeScanService(struct uhuru ** uhuru, PUSER_SCAN_CONTEXT Context) {

	uhuru_error * uh_error = NULL;
	HRESULT hres = S_OK;			
	

	*uhuru = uhuru_open(&uh_error);
	if (*uhuru == NULL) {	
		uhLog("[-] Error :: uhuru_open failed\n");
		return -1;
	}

	uhLog("[+] Debug :: uhuru struct initialized successfully\n");

	//  Initialize scan listening threads.
	// hres = UserScanInit(&userScanCtx);


	return 0;
}

HRESULT UserScanFinalize(_In_  PUSER_SCAN_CONTEXT Context) {

	HRESULT hres = S_OK;
	HANDLE hScanThreads[USER_SCAN_THREAD_COUNT] = {0};
	int i = 0;
	
	if (Context->ScanThreadCtxes == NULL) {
		uhLog("[-] Warning :: UserScanFinalize :: NULL Scan Thread Contexts\n");
		//hres = E_POINTER;
		return hres;
	}

	uhLog("[+] Debug :: UserScanFinalize :: Finalizing threads....\n");

	// Tell all scannig threads that the program is going to exit.	
	Context->Finalized = TRUE;
	
	uhLog("[DEBUG1]...\n");
	for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {
		//printf("Thread id : %d\n", Context->ScanThreadCtxes[i].ThreadId);		
		Context->ScanThreadCtxes[i].Aborted = TRUE;			 
	}	

	// Wake up the listeing thread if it is waiting for messag via GetQueuedCompletionStatus()
	CancelIoEx(Context->ConnectionPort, NULL);	
	
	// Wait for all scan threads to complete cancellation, so we will able to close the connection port.
	for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {
		if (Context->ScanThreadCtxes[i].Handle == INVALID_HANDLE_VALUE || Context->ScanThreadCtxes[i].Handle == NULL ) {
			uhLog("[Error] :: UserScanFinalize :: NULL Thread Handle\n");
		}
		hScanThreads[i] = Context->ScanThreadCtxes[i].Handle;
	}

	WaitForMultipleObjects(USER_SCAN_THREAD_COUNT,hScanThreads,TRUE,INFINITE );	
	
	uhLog("[+] Debug :: UserScanFinalize :: Closing connection port.\n");


	// close all scan ports.

	if ( (Context->ConnectionPort != INVALID_HANDLE_VALUE && Context->ConnectionPort != NULL ) && !CloseHandle(Context->ConnectionPort)) {
		uhLog("[-] Error :: UserScanFinalize :: Closehandle connection port failed !\n");
		hres = HRESULT_FROM_WIN32(GetLastError());		
	}
	Context->ConnectionPort = NULL;

	if (!CloseHandle(Context->Completion)) {
		uhLog("[-] Error :: UserScanFinalize :: CloseHandle completion port failed !\n");
		hres = HRESULT_FROM_WIN32(GetLastError());
	}

	Context->Completion = NULL;

	//  Clean up scan thread contexts
	for (i = 0; i < USER_SCAN_THREAD_COUNT; i++) {

		if (Context->ScanThreadCtxes[i].Handle && CloseHandle(Context->ScanThreadCtxes[i].Handle) == FALSE ) {
			uhLog("[-] Error :: UhuruSvc!UserScanFinalize :: CloseHandle failed for thread %d :: GLE=%03d. \n",i,GetLastError());
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
	char * tmp;
	

	if (DeviceFileName == NULL) {
		uhLog("[-] Error :: ConvertDeviceNameToMsDosName :: null parameter DeviceName\n");
		return NULL;
	}

	// Get the list of the logical drives.
	len = GetLogicalDriveStringsA(BUFSIZE,deviceDosName);
	if (len == 0) {
		uhLog("[-] Error :: ConvertDeviceNameToMsDosName!GetLogicalDriveStrings() failed ::  error code = 0x%03d\n",GetLastError());
		return NULL;
	}
		
	
	tmp = deviceDosName;

	do {

		//printf("[+] Debug :: deviceDosName = %s\n",tmp);

		// Get the device letter without the backslash (Ex: C:).
		memcpy_s(deviceLetter,2,tmp,2);
		
		if (!QueryDosDeviceA(deviceLetter, deviceNameQuery, BUFSIZE)) {
			uhLog("[-] Error :: QueryDosDeviceA() failed ::  error code = 0x%03d\n",GetLastError());
			return NULL;
		}
		//printf("[+] Debug :: DeviceName = %s ==> %s\n",deviceNameQuery,deviceLetter);
		if (deviceNameQuery == NULL) {
			uhLog("[-] Error :: ConvertDeviceNameToMsDosName :: QueryDosDeviceA() failed ::  deviceNameQuery is NULL\n",GetLastError());
		}

		if (deviceNameQuery != NULL && strstr(DeviceFileName,deviceNameQuery) != NULL) {
			//printf("[+] Debug :: FOUND DeviceName = %s ==> %s\n",deviceNameQuery,deviceLetter);

			len2 = strnlen_s(deviceNameQuery, MAX_PATH_SIZE);
			len = strnlen_s(DeviceFileName, MAX_PATH_SIZE) - len2 + 3;					
			
			deviceDosFilename = (char*)calloc(len+1,sizeof(char) );
			deviceDosFilename[len] = '\0';

			memcpy_s(deviceDosFilename, len, tmp, 3);
			memcpy_s(deviceDosFilename+2, len, DeviceFileName+len2 , len-1);
			
			bFound = TRUE;

		}

		// got to the next device name.
		while (*tmp++);
		//printf("[+] Debug :: next device name = %s\n",tmp);
		
		
	} while (bFound == FALSE && *tmp);


	if (bFound == FALSE) {
		return NULL;
	}
	

	return deviceDosFilename;
}

int closeScanService(struct uhuru * uhuru) {

	uhuru_error * uh_error = NULL;

	if (uhuru != NULL) {
		uhuru_close(uhuru, &uh_error);
		uhLog("[+] Debug :: uhuru struct closed successfully!\n");
	}
	else {
		uhLog("[-] Warning :: uhuru global struct is NULL !\n");
	}

	return 0;

}