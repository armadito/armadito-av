#include "callbacks.h"
#include "Struct.h"
#include "communication.h"
#include "UhuruGuard.h"

extern PFLT_PORT gClientComPort;
//extern PEPROCESS gScanProcess;

const CHAR * PrintUhuruScanResult(SCAN_RESULT res) {

	switch (res) {

			case NONE:
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
			case TIMEOUT:
				return "[TIMEOUT]";
				break;
			case NOT_CONNECTED:
				return "[NOT_CONNECTED]";
				break;
			default:
				return "[ERROR_DEFAULT]";
				break;
		}
	
}

// This function launch a Scan with the analysis process.
// This function is a FAKE scan.
SCAN_RESULT LaunchFileAnalysis(_In_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects) {

	NTSTATUS ntStatus;
	//SCAN_RESULT res = NONE;
	PFLT_FILE_NAME_INFORMATION FileNameInformation = NULL;	
	//UNICODE_STRING malName;
	SCAN_RESULT answer = NONE;
	
	
	UNREFERENCED_PARAMETER( FltObjects );

	if (gClientComPort == NULL) {
		//DbgPrint("[-] Warning :: UhuruGuard!LaunchFileAnalysis :: Scan service not connected yet !! \n");
		return NOT_CONNECTED;
	}

#if 1

	__try {

		// For test purposes.	
		//RtlInitUnicodeString(&malName,L"malware.txt"); // Free unicode string.
		

		// Retrieve file name informations.
		ntStatus = FltGetFileNameInformation(Data,FLT_FILE_NAME_NORMALIZED|FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInformation);
		if (!NT_SUCCESS(ntStatus)) {
			DbgPrint("[-] Error :: UhuruGuard!LaunchFileAnalysis :: FltGetFileNameInformation() routine failed :: 0x%x \n",ntStatus);
			//answer = NONE;
			__leave;			
		}

		ntStatus = FltParseFileNameInformation(FileNameInformation);
		if (!NT_SUCCESS(ntStatus)) {
			
			DbgPrint("[-] Error :: UhuruGuard!LaunchFileAnalysis :: FltParseFileNameInformation() routine failed :: 0x%x \n", ntStatus);
			//res = NONE;
			__leave;
			//FltReleaseFileNameInformation(FileNameInformation);
			//FileNameInformation = NULL;
			//return FLT_POSTOP_FINISHED_PROCESSING;
		}

		//-----------------------------------------
		// Send scan order to the scan service.
		ntStatus = SendScanOrder(FltObjects->Filter, &FileNameInformation->Name, &answer);
		if (!NT_SUCCESS(ntStatus)) {			
			DbgPrint("[-] Error :: UhuruGuard!LaunchFileAnalysis :: SendScanOrder() failed :: 0x%x \n", ntStatus);
			answer = UHURU_IERROR;
			__leave;			
		}

		// MALWARE detected by the scan.
		//if (answer == UHURU_MALWARE)
		//	DbgPrint("[i] WARNING :: UhuruGuard!LaunchFileAnalysis :: MALWARE DETECTED :: %wZ.\n",FltObjects->FileObject->FileName);

		/*switch (answer) {

			case NONE:
				DbgPrint("[+] Debug :: UhuruGuard!LaunchFileAnalysis :: %wZ :: [NONE]\n",FltObjects->FileObject->FileName);
				break;
			case UHURU_UNDECIDED:
				DbgPrint("[+] Debug :: UhuruGuard!LaunchFileAnalysis :: %wZ :: [UHURU_UNDECIDED]\n",FltObjects->FileObject->FileName);					
				break;
			case UHURU_CLEAN:
				DbgPrint("[+] Debug :: UhuruGuard!LaunchFileAnalysis :: %wZ :: [UHURU_CLEAN]\n",FltObjects->FileObject->FileName);
				break;
			case UHURU_UNKNOWN_FILE_TYPE:
				DbgPrint("[+] Debug :: UhuruGuard!LaunchFileAnalysis :: %wZ :: [UHURU_UNKNOWN_FILE_TYPE]\n",FltObjects->FileObject->FileName);
				break;
			case UHURU_EINVAL:
				DbgPrint("[+] Debug :: UhuruGuard!LaunchFileAnalysis :: %wZ :: [UHURU_EINVAL]\n",FltObjects->FileObject->FileName);
				break;
			case UHURU_IERROR:
				DbgPrint("[+] Debug :: UhuruGuard!LaunchFileAnalysis :: %wZ :: [UHURU_IERROR]\n",FltObjects->FileObject->FileName);
				break;
			case UHURU_SUSPICIOUS:
				DbgPrint("[+] Debug :: UhuruGuard!LaunchFileAnalysis :: %wZ :: [UHURU_SUSPICIOUS]\n",FltObjects->FileObject->FileName);
				break;
			case UHURU_WHITE_LISTED:
				DbgPrint("[+] Debug :: UhuruGuard!LaunchFileAnalysis :: %wZ :: [UHURU_WHITE_LISTED]\n",FltObjects->FileObject->FileName);
				break;
			case UHURU_MALWARE:
				DbgPrint("[+] WARNING :: UhuruGuard!LaunchFileAnalysis :: %wZ :: [UHURU_MALWARE]\n",FltObjects->FileObject->FileName);
				break;
			case TIMEOUT:
				DbgPrint("[+] Debug :: UhuruGuard!LaunchFileAnalysis :: %wZ :: [TIMEOUT]\n",FltObjects->FileObject->FileName);
				break;
			default:
				DbgPrint("[+] Debug :: UhuruGuard!LaunchFileAnalysis :: %wZ :: [ERROR_DEFAULT] \n",FltObjects->FileObject->FileName);
				break;

		}*/
		//DbgPrint("[-] Warning :: UhuruGuard!LaunchFileAnalysis :: FltSendMessage() returned with timeout status !! \n");

		//-----------------------------------------
		/* fake scan.
		if ( RtlEqualUnicodeString(&FileNameInformation->FinalComponent,&malName,FALSE) == TRUE ) {	
			res = UHURU_MALWARE;
		}
		else {
			res = UHURU_CLEAN;
		}
		/************************************************/

	}
	__finally {

		if (FileNameInformation != NULL) {
			FltReleaseFileNameInformation(FileNameInformation);
			FileNameInformation = NULL;
		}
		
		/*if (&malName != NULL) {
			RtlFreeUnicodeString(&malName);
		}*/

	}
	

#endif

	return answer;
}


/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/


FLT_PREOP_CALLBACK_STATUS
PreOperationIrpCreate (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    )
/*++

Routine Description:

    This routine is a pre-operation dispatch routine for this miniFilter.

    This is non-pageable because it could be called on the paging path

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The context for the completion routine for this
        operation.

Return Value:

    The return value is the status of the operation.

--*/
{
  
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN bIsScanProcess = FALSE;
	//ULONG ProcessId =0;
	UNREFERENCED_PARAMETER( Data );
    //UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

	// Check if the request come from the scan process.
	status = IsFromTheAnalysisProcess(FltGetRequestorProcess(Data),&bIsScanProcess);
	if (!NT_SUCCESS(status)){
		//DbgPrint("[i] Error :: UhuruGuard!PreOperationIrpCreate :: IsFromTheAnalysisProcess failed :: status = 0x%x :: %d\n", status,status);
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	if (bIsScanProcess == TRUE) {
		//ProcessId = FltGetRequestorProcessId(Data);
		//DbgPrint("[-] Warning :: UhuruGuard!PreOperationIrpCreate :: [%d] Ignoring Analysis process request !! :: %wZ\n",ProcessId,FltObjects->FileObject->FileName);
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	


	// If the caller thread is a system thread, do not call the post operation callback.
	if (PsIsSystemThread(Data->Thread) == TRUE) {
		//DbgPrint("[i] Debug :: UhuruGuard!PreOperationIrpCreate :: System thread caller\n");
		return FLT_PREOP_SUCCESS_NO_CALLBACK ;
	}
    
	//
    //  Directory opens don't need to be scanned.
    //

    if (FlagOn( Data->Iopb->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {        
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    //
    //  Skip pre-rename operations which always open a directory.
    //

    if ( FlagOn( Data->Iopb->OperationFlags, SL_OPEN_TARGET_DIRECTORY )) {
        
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }
    
    //
    //  Skip paging files.
    //

    if (FlagOn( Data->Iopb->OperationFlags, SL_OPEN_PAGING_FILE )) {
       
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    //
    //  Skip scanning DASD opens 
    //
    
    if (FlagOn( FltObjects->FileObject->Flags, FO_VOLUME_OPEN )) {

        return FLT_PREOP_SUCCESS_NO_CALLBACK;      
    } 


    // returns FLT_PREOP_SUCCESS_WITH_CALLBACK.
    // This passes the request down to the next miniFilter in the chain.

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}


/*
#define PASSIVE_LEVEL 0                 // Passive release level
#define LOW_LEVEL 0                     // Lowest interrupt level
#define APC_LEVEL 1                     // APC interrupt level
#define DISPATCH_LEVEL 2                // Dispatcher level
#define CMCI_LEVEL 5  
*/
_IRQL_requires_max_(PASSIVE_LEVEL)		// This is only the case for a post create operation callback.
_IRQL_requires_same_
FLT_POSTOP_CALLBACK_STATUS
PostOperationIrpCreate(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags)
/*++

Routine Description:

    This routine is a pre-operation dispatch routine for this miniFilter.

    This is non-pageable because it could be called on the paging path

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The context for the completion routine for this
        operation.

Return Value:

    The return value is the status of the operation.

--*/
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
	FLT_POSTOP_CALLBACK_STATUS retStatus = FLT_POSTOP_FINISHED_PROCESSING;
	BOOLEAN bIsDir = FALSE;
	//BOOLEAN bIsScanProcess = FALSE;
	//BOOLEAN bleave = FALSE;
	PFILE_CONTEXT FileContext = NULL;
	SCAN_RESULT scanRes;

	UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( CompletionContext );

	// Check if the operation succeeded or not.
	if (!NT_SUCCESS(Data->IoStatus.Status)) {		
		return FLT_POSTOP_FINISHED_PROCESSING;
	}

	// Ignore Volume Requests.
	if (FlagOn(FltObjects->FileObject->Flags, FO_VOLUME_OPEN)) {
		DbgPrint("[-] Warning :: UhuruGuard!PostOperationIrpCreate :: Ignore VOLUME_OPEN requests.\n");
		return FLT_POSTOP_FINISHED_PROCESSING;			
	}


	__try {

		// Check if the request come from the scan process.
		/*
		ntStatus = IsFromTheAnalysisProcess(FltGetRequestorProcess(Data),&bIsScanProcess);
		if (!NT_SUCCESS(ntStatus)){
			DbgPrint("[i] Error :: UhuruGuard!PostOperationIrpCreate :: IsFromTheAnalysisProcess failed :: status = 0x%x :: %d\n", ntStatus,ntStatus);
			__leave;
		}

		if (bIsScanProcess == TRUE) {
			DbgPrint("[-] Warning :: UhuruGuard!PosOperationIrpCreate :: !! Ignoring Analysis process request !! :: %wZ\n",FltObjects->FileObject->FileName);
			__leave;
		}*/

		// Ignore Directory Open Creation.
		ntStatus = FltIsDirectory(FltObjects->FileObject, FltObjects->Instance, &bIsDir);
		if (!NT_SUCCESS(Data->IoStatus.Status)) {
			DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpCreate :: FltIsDirectory() routine failed !! \n");
			__leave;
		}

		if (bIsDir == TRUE) {
			//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: Ignoring Directory file operation \n");
			__leave;
		}

#if 0
		// Skip encrypted files :: source avscan project.
		if (!(FlagOn(desiredAccess, FILE_WRITE_DATA)) && 
        !(FlagOn(desiredAccess, FILE_READ_DATA)) ) {
        
			BOOLEAN encrypted = FALSE;
			status = AvGetFileEncrypted( FltObjects->Instance,
										 FltObjects->FileObject,
										 &encrypted );
			if (!NT_SUCCESS( status )) {

				AV_DBG_PRINT( AVDBG_TRACE_ROUTINES,
					  ("[AV] AvPostCreate: AvGetFileEncrypted FAILED!! \n0x%x\n", status) );
			}
			if (encrypted) {
        
				return FLT_POSTOP_FINISHED_PROCESSING;
			}
		}
#endif

		// 
		//Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess == GENERIC_READ;
		//Data->Iopb->Parameters.Create.SecurityContext->AccessState->OriginalDesiredAccess == GENERIC_READ;		


		/* Create a context to the file. */
		ntStatus = FltGetFileContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, (PFLT_CONTEXT *)&FileContext);

		if (ntStatus == STATUS_NOT_SUPPORTED) {			
			__leave;
		}

		// If there is no context set for this file.
		if (ntStatus == STATUS_NOT_FOUND) {

			//
			//	Test if the file is executable or writable. If it is not the case, we pass through the operation.
			//	It's much more about system optimization.
			//
			if( ! FlagOn(Data->Iopb->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess, FILE_EXECUTE) &&
				! FlagOn(Data->Iopb->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess, FILE_WRITE_DATA) &&
				! FlagOn(Data->Iopb->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess, FILE_READ_DATA) 
			){
				__leave;
			}

			ntStatus = FltAllocateContext(FltObjects->Filter, FLT_FILE_CONTEXT, sizeof(FILE_CONTEXT), NonPagedPool, (PFLT_CONTEXT)&FileContext);
			if (!NT_SUCCESS(ntStatus)) {
				DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpCreate :: FltAllocateContext routine failed\n");
				__leave;
			}

			//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: Context allocated :: %wZ.\n",FltObjects->FileObject->FileName);
			RtlZeroMemory(FileContext, sizeof(FILE_CONTEXT));

			// init scan result.
			FileContext->scanResult = NONE;

			ntStatus = FltSetFileContext(FltObjects->Instance,FltObjects->FileObject,FLT_SET_CONTEXT_KEEP_IF_EXISTS, FileContext, NULL );
			if (!NT_SUCCESS(ntStatus)) {
				DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpCreate :: FltSetFileContext routine failed\n");
				__leave;
			}

			// For file creation, scan the file at cleanup.
			if (FlagOn(Data->IoStatus.Information, FILE_CREATED) ) {
				uhDbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: FILE_CREATED flag :: %wZ\n", FltObjects->FileObject->FileName);
				__leave;
			}

		}

		
		// if the file has already been analyzed do no analyze it twice.
		if ( FileContext->scanResult == NONE ) {
			// Launch file analysis.
			scanRes = LaunchFileAnalysis(Data,FltObjects);
			FileContext->scanResult = scanRes;
			// Print scan result:
			if (FileContext->scanResult == UHURU_MALWARE || FileContext->scanResult == UHURU_CLEAN)
				uhDbgPrint("[i] INFO :: UhuruGuard!PostOperationIrpCreate ::[%d]:: %wZ => %s\n",FltGetRequestorProcessId(Data), FltObjects->FileObject->FileName,PrintUhuruScanResult(FileContext->scanResult));
		}
		

		/*
		if (FileContext->scanResult == UHURU_IERROR) {
			DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpCreate :: LaunchFileAnalysis failed for file :: %wZ.\n",FltObjects->FileObject->FileName);	;
			__leave;
		}
		*/

		/*
		// Display filename informations.
		DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: Volume = %wZ \n",FileNameInformation->Volume);
		DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: ParentDir = %wZ \n",FileNameInformation->ParentDir);
		DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: Name = %wZ \n",FileNameInformation->Name);
		DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: FinalComponent = %wZ \n",FileNameInformation->FinalComponent);	
		DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: File created :: %wZ.\n",FltObjects->FileObject->FileName);
		*/

		// If the file is detected as a malware, cancel file operation.
		if (FileContext->scanResult == UHURU_MALWARE ) {

			DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: MALWARE DETECTED :: %wZ.\n",FltObjects->FileObject->FileName);	
		
			// Cancel file operation.
			// https://msdn.microsoft.com/en-us/library/windows/hardware/ff540223%28v=vs.85%29.aspx
			// 1. Call FltCancelFileOpen
			FltCancelFileOpen(FltObjects->Instance, FltObjects->FileObject);

			// 2. Set the callback data structure's IoStatus.status field to the final NTSTATUS value for the operation.
			Data->IoStatus.Status = STATUS_ACCESS_DENIED;

			// 3. Set the callback data structure IoStatus.Information field to zero.
			Data->IoStatus.Information = 0;
					
			// 4. Return FLT_POSTOP_FINISHED_PROCESSING
			//return FLT_POSTOP_FINISHED_PROCESSING;
			__leave; // free Unicode string in __finally section.
	
		}

		// remove this call
		/*if (FileContext != NULL) {
			FltReleaseContext((PFLT_CONTEXT)FileContext);
			FileContext = NULL;
		}*/
		//btry = TRUE;


	}
	__finally {

		// Free all allocated struct and mem.
		if (FileContext != NULL) {
			FltReleaseContext((PFLT_CONTEXT)FileContext);
		}

	}

    return retStatus;
}


FLT_POSTOP_CALLBACK_STATUS
PostOperationIrpWrite(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags)
/*++

Routine Description:

    This routine is a pre-operation dispatch routine for this miniFilter.

    This is non-pageable because it could be called on the paging path

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The context for the completion routine for this
        operation.

Return Value:

    The return value is the status of the operation.

--*/
{
    NTSTATUS ntStatus, retstatus  = FLT_POSTOP_FINISHED_PROCESSING;
	PFILE_CONTEXT FileContext = NULL;
	BOOLEAN bIsDir = FALSE;
	BOOLEAN bIsScanProcess = FALSE;

	//UNREFERENCED_PARAMETER( Data );
	UNREFERENCED_PARAMETER( Flags );
    //UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

	__try {

		// If the size the content to write is NULL, then leave.
		if (Data->Iopb->Parameters.Write.Length == 0) {
			__leave;
		}

		if (!NT_SUCCESS(Data->IoStatus.Status) ) {
			if (!FLT_IS_FASTIO_OPERATION(Data))
				DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpWrite :: WRITE OPERATION failed !! 0x%x :: %wZ \n",Data->IoStatus.Status,FltObjects->FileObject->FileName);
			__leave;
		}

		// Test if the requestor process is the analysis service.
		ntStatus = IsFromTheAnalysisProcess(FltGetRequestorProcess(Data),&bIsScanProcess);
		if (!NT_SUCCESS(ntStatus)){
			//DbgPrint("[i] Error :: UhuruGuard!PreOperationIrpCleanup :: IsFromTheAnalysisProcess failed :: status = 0x%x :: %d\n", status,status);
			__leave;
		}

		if (bIsScanProcess == TRUE) {
			//DbgPrint("[+] Debug :: UhuruGuard!PostOperationIrpWrite :: !! Ignoring Analysis process request !! :: %wZ\n",FltObjects->FileObject->FileName);
			__leave;
		}

		// Ignore Directory Open Creation.
		ntStatus = FltIsDirectory(FltObjects->FileObject, FltObjects->Instance, &bIsDir);
		if (!NT_SUCCESS(ntStatus)) {
			DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpWrite :: FltIsDirectory() routine failed !! 0x%x ::\n",ntStatus);
			__leave;
		}

		if (bIsDir == TRUE) {
			//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: Ignoring Directory file operation \n");
			__leave;
		}

		ntStatus = FltGetFileContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, (PFLT_CONTEXT *)&FileContext);

		
		if (ntStatus == STATUS_NOT_SUPPORTED) {			
			__leave;
		} else if (ntStatus == STATUS_NOT_FOUND) {

			// If there is no context found, then set a context to this file.
			ntStatus = FltAllocateContext(FltObjects->Filter, FLT_FILE_CONTEXT, sizeof(FILE_CONTEXT), NonPagedPool, (PFLT_CONTEXT)&FileContext);
			if (!NT_SUCCESS(ntStatus)) {
				DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpWrite :: FltAllocateContext routine failed\n");
				__leave;
			}

			//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: Context allocated :: %wZ.\n",FltObjects->FileObject->FileName);
			RtlZeroMemory(FileContext, sizeof(FILE_CONTEXT));

			// init scan result.
			FileContext->scanResult = NONE;

			ntStatus = FltSetFileContext(FltObjects->Instance,FltObjects->FileObject,FLT_SET_CONTEXT_KEEP_IF_EXISTS, FileContext, NULL );
			if (!NT_SUCCESS(ntStatus)) {
				DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpWrite :: FltSetFileContext routine failed\n");
				__leave;
			}

		}
		else if (NT_SUCCESS(ntStatus)) {	// If a context is already set for this file.
			
			if (FileContext->scanResult == UHURU_CLEAN)
				//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpWrite :: Writing in MARKED file %s :: %wZ\n",PrintUhuruScanResult(FileContext->scanResult),FltObjects->FileObject->FileName);
			
			// If the file was no detected as a malware, then rescan it later.
			if (FileContext->scanResult != UHURU_MALWARE) {
				FileContext->scanResult = NONE; // Reset the scan result.
			}
			else {
				// Cancel operation. in the postoperation.
				// TODO;
				DbgPrint("[+] Debug :: UhuruGuard!PostOperationIrpWrite :: Writing in UHURU_MALWARE marked file :: %wZ\n",FltObjects->FileObject->FileName);
				__leave;

			}
			
			__leave; //return FLT_POSTOP_FINISHED_PROCESSING;

		}
		
		
		//FltReleaseContext((PFLT_CONTEXT)FileContext);
		//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCleanup :: ::Context realeased for file %wZ\n",FltObjects->FileObject->FileName);


	}
	__finally {

		if (FileContext != NULL) {
			//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpWrite :: Freeing context in Write callback :: %wZ\n",FltObjects->FileObject->FileName);
			FltReleaseContext((PFLT_CONTEXT)FileContext);			
		}

	}


    return retstatus;
}




FLT_PREOP_CALLBACK_STATUS
PreOperationIrpCleanup(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
) {

	NTSTATUS ntStatus = STATUS_SUCCESS;
	FLT_PREOP_CALLBACK_STATUS status = FLT_PREOP_SUCCESS_NO_CALLBACK;
	BOOLEAN bIsScanProcess = FALSE;
	BOOLEAN bIsDirectory = FALSE;
	PFILE_CONTEXT FileContext = NULL;
	SCAN_RESULT scanResult = NONE;

	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(CompletionContext);

	__try {

		
		// Test if the requestor process is the analysis service.
		ntStatus = IsFromTheAnalysisProcess(FltGetRequestorProcess(Data),&bIsScanProcess);
		if (!NT_SUCCESS(ntStatus)){
			//DbgPrint("[i] Error :: UhuruGuard!PreOperationIrpCleanup :: IsFromTheAnalysisProcess failed :: status = 0x%x :: %d\n", status,status);
			__leave;
		}

		if (bIsScanProcess == TRUE) {
			//DbgPrint("[+] Debug :: UhuruGuard!PreOperationIrpCleanup :: !! Ignoring Analysis process request !! :: %wZ\n",FltObjects->FileObject->FileName);
			__leave;
		}

		// Ignore directories
		ntStatus = FltIsDirectory(FltObjects->FileObject, FltObjects->Instance, &bIsDirectory);
		if (!NT_SUCCESS(Data->IoStatus.Status)) {
			DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpCleanup :: FltIsDirectory() routine failed !! \n");
			__leave;
		}

		if (bIsDirectory == TRUE)
			__leave;

		if(FlagOn(FltObjects->FileObject->Flags, FO_VOLUME_OPEN)){
			__leave;
		}

				

		
		//DbgPrint("[+] Debug :: UhuruGuard!PreOperationIrpCleanup :: %wZ\n", FltObjects->FileObject->FileName);
		// Get the file context.
		ntStatus = FltGetFileContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, (PFLT_CONTEXT *)&FileContext);
		if (!NT_SUCCESS(ntStatus)){			
			__leave;
		}

		// Test if the file is marked for deletion; then release and delete the context linked to the file.
		if (FlagOn(FltObjects->FileObject->Flags,FO_DELETE_ON_CLOSE) || FltObjects->FileObject->DeletePending == TRUE ) {
			status = FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}

		// Test if the file has already been scanned.
		if (FileContext->scanResult != NONE && FileContext->scanResult != TIMEOUT && FileContext->scanResult != UHURU_UNKNOWN_FILE_TYPE ) {
			//DbgPrint("[+] Debug :: UhuruGuard!PreOperationIrpCleanup :: File Already Scanned  :: %wZ :: %s\n", FltObjects->FileObject->FileName, PrintUhuruScanResult(FileContext->scanResult));
			__leave;
		}
			

		// Scan the file through the analysis service.
		scanResult = LaunchFileAnalysis(Data,FltObjects);
		FileContext->scanResult = scanResult;

		//if (FileContext->scanResult == UHURU_MALWARE || FileContext->scanResult == UHURU_CLEAN)
		uhDbgPrint("[i] INFO :: UhuruGuard!PreOperationIrpCleanup ::[%d]:: %wZ => %s\n",FltGetRequestorProcessId(Data),FltObjects->FileObject->FileName,PrintUhuruScanResult(FileContext->scanResult));


		// if the file has been detected has malware...
		if (FileContext->scanResult == UHURU_MALWARE) {
			//status = FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}
		

	}
	__finally {

		if(FileContext != NULL){			
			FltReleaseContext(FileContext);			
		}

		
		//status = FLT_PREOP_SUCCESS_WITH_CALLBACK;

	}


	return status;
}


// TODO ::  DO not launch the analysis in this Callback. Scan in the preoperation callback ( because of FltGetFilenameInformation).
FLT_POSTOP_CALLBACK_STATUS
PostOperationIrpCleanup(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags)
/*++

Routine Description:

    This routine is a pre-operation dispatch routine for this miniFilter.

    This is non-pageable because it could be called on the paging path

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The context for the completion routine for this
        operation.

Return Value:

    The return value is the status of the operation.

--*/
{
    NTSTATUS ntStatus, retstatus  = FLT_POSTOP_FINISHED_PROCESSING;	
	PFILE_CONTEXT FileContext = NULL;
	//SCAN_RESULT scanRes = NONE;
	

	UNREFERENCED_PARAMETER( Flags );  
	//UNREFERENCED_PARAMETER( FltObjects ); 
    UNREFERENCED_PARAMETER( CompletionContext );

	__try {

		ntStatus = FltGetFileContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, (PFLT_CONTEXT *)&FileContext);

		if (!NT_SUCCESS(ntStatus)) {			
			__leave; //return FLT_POSTOP_FINISHED_PROCESSING;
		}

		uhDbgPrint("[+] Debug :: UhuruGuard!PostOperationIrpCleanup :: Cleaning File Context :: %wZ => %s\n", FltObjects->FileObject->FileName,PrintUhuruScanResult(FileContext->scanResult));
		
		
		//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCleanup :: ::Context realeased for file %wZ\n",FltObjects->FileObject->FileName);

		// maybe use FltDeleteFileContext( ) instead of FltDeleteContext();		
			
		if (FileContext != NULL && FileContext->scanResult != UHURU_MALWARE ) {
			FltReleaseContext((PFLT_CONTEXT)FileContext);
			FltDeleteContext((PFLT_CONTEXT)FileContext);
			FileContext = NULL;
		}

	}
	__finally {

	}

    return retstatus;
}