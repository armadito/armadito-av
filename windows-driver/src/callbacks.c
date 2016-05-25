#include "callbacks.h"
#include "communication.h"
#include "ArmaditoGuard.h"

extern PFLT_PORT gClientComPort;
//extern PEPROCESS gScanProcess;

const CHAR * PrintArmaditoScanResult(SCAN_RESULT res) {

	switch (res) {

			case NONE:
				return "[NONE]";				
				break;
			case ARMADITO_UNDECIDED:
				return "[ARMADITO_UNDECIDED]";				
				break;
			case ARMADITO_CLEAN:
				return "[ARMADITO_CLEAN]";
				break;
			case ARMADITO_UNKNOWN_FILE_TYPE:
				return "[ARMADITO_UNKNOWN_FILE_TYPE]";
				break;
			case ARMADITO_EINVAL:
				return "[ARMADITO_EINVAL]";
				break;
			case ARMADITO_IERROR:
				return "[ARMADITO_IERROR]";
				break;
			case ARMADITO_SUSPICIOUS:
				return "[ARMADITO_SUSPICIOUS]";
				break;
			case ARMADITO_WHITE_LISTED:
				return "[ARMADITO_WHITE_LISTED]";
				break;
			case ARMADITO_MALWARE:
				return "[ARMADITO_MALWARE]";
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
	UNICODE_STRING quarantineDir;
	SCAN_RESULT answer = NONE;
	
	
	UNREFERENCED_PARAMETER( FltObjects );
	UNREFERENCED_PARAMETER( quarantineDir );

	if (gClientComPort == NULL) {
		//DbgPrint("[-] Warning :: ArmaditoGuard!LaunchFileAnalysis :: Scan service not connected yet !! \n");
		return NOT_CONNECTED;
	}

#if 1

	__try {

		// For test purposes.	
		//RtlInitUnicodeString(&malName,L"malware.txt"); // Free unicode string.
		

		// Retrieve file name informations.
		ntStatus = FltGetFileNameInformation(Data,FLT_FILE_NAME_NORMALIZED|FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInformation);
		if (!NT_SUCCESS(ntStatus)) {
			DbgPrint("[-] Error :: ArmaditoGuard!LaunchFileAnalysis :: FltGetFileNameInformation() routine failed :: 0x%x \n",ntStatus);
			//answer = NONE;
			__leave;			
		}

		ntStatus = FltParseFileNameInformation(FileNameInformation);
		if (!NT_SUCCESS(ntStatus)) {
			
			DbgPrint("[-] Error :: ArmaditoGuard!LaunchFileAnalysis :: FltParseFileNameInformation() routine failed :: 0x%x \n", ntStatus);
			//res = NONE;
			__leave;
			//FltReleaseFileNameInformation(FileNameInformation);
			//FileNameInformation = NULL;
			//return FLT_POSTOP_FINISHED_PROCESSING;
		}

		// Deny access to Quarantine folder		
		/*RtlInitUnicodeString(&quarantineDir,QUARANTINE_DIR); // Free unicode string.
		if (RtlEqualUnicodeString(&FileNameInformation->ParentDir, &quarantineDir, FALSE) == TRUE) {
			answer = ARMADITO_MALWARE;
			__leave;
		}
		*/

		//-----------------------------------------
		// Send scan order to the scan service.
		ntStatus = SendScanOrder(FltObjects->Filter, &FileNameInformation->Name, &answer);
		if (!NT_SUCCESS(ntStatus)) {			
			DbgPrint("[-] Error :: ArmaditoGuard!LaunchFileAnalysis :: SendScanOrder() failed :: 0x%x \n", ntStatus);
			answer = ARMADITO_IERROR;
			__leave;			
		}

		
		//-----------------------------------------
		/* fake scan.
		if ( RtlEqualUnicodeString(&FileNameInformation->FinalComponent,&malName,FALSE) == TRUE ) {	
			res = ARMADITO_MALWARE;
		}
		else {
			res = ARMADITO_CLEAN;
		}
		/************************************************/

	}
	__finally {

		if (FileNameInformation != NULL) {
			FltReleaseFileNameInformation(FileNameInformation);
			FileNameInformation = NULL;
		}
		

		/*if (&quarantineDir != NULL) {
			RtlFreeUnicodeString(&quarantineDir);
		}*/

	}
	

#endif

	return answer;
}


/*++

Routine Description:

    This routine obtains the File ID and saves it in the stream context.

Arguments:

    Instance - Opaque filter pointer for the caller. This parameter is required and cannot be NULL.
    
    FileObject - File object pointer for the file. This parameter is required and cannot be NULL.

    Encrypted - Pointer to a boolean indicating if this file is encrypted or not. This is the output.

Return Value:

    Returns statuses forwarded from FltQueryInformationFile.

--*/
NTSTATUS IsFileEncrypted (
    _In_   PFLT_INSTANCE Instance,
    _In_   PFILE_OBJECT FileObject,
    _Out_  PBOOLEAN  Encrypted
    ){
    NTSTATUS status = STATUS_SUCCESS;
    FILE_BASIC_INFORMATION basicInfo;

    //
    //  Querying for basic information to get encryption.
    //

    status = FltQueryInformationFile( Instance,
                                      FileObject,
                                      &basicInfo,
                                      sizeof(FILE_BASIC_INFORMATION),
                                      FileBasicInformation,
                                      NULL );

    if (NT_SUCCESS( status )) {

        *Encrypted = BooleanFlagOn( basicInfo.FileAttributes, FILE_ATTRIBUTE_ENCRYPTED );
    }

    return status;
}



/*************************************************************************
    ArmaditoGuard callback routines.
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
		//DbgPrint("[i] Error :: ArmaditoGuard!PreOperationIrpCreate :: IsFromTheAnalysisProcess failed :: status = 0x%x :: %d\n", status,status);
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	if (bIsScanProcess == TRUE) {
		//ProcessId = FltGetRequestorProcessId(Data);
		//DbgPrint("[-] Warning :: ArmaditoGuard!PreOperationIrpCreate :: [%d] Ignoring Analysis process request !! :: %wZ\n",ProcessId,FltObjects->FileObject->FileName);
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	


	// If the caller thread is a system thread, do not call the post operation callback.
	if (PsIsSystemThread(Data->Thread) == TRUE) {
		//DbgPrint("[i] Debug :: ArmaditoGuard!PreOperationIrpCreate :: System thread caller\n");
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
	BOOLEAN encrypted = FALSE;
	ACCESS_MASK desiredAccess = Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess;
	

	UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( CompletionContext );

	// Check if the operation succeeded or not.
	if (!NT_SUCCESS(Data->IoStatus.Status)) {		
		return FLT_POSTOP_FINISHED_PROCESSING;
	}

	// Ignore Volume Requests.
	if (FlagOn(FltObjects->FileObject->Flags, FO_VOLUME_OPEN)) {
		DbgPrint("[-] Warning :: ArmaditoGuard!PostOperationIrpCreate :: Ignore VOLUME_OPEN requests.\n");
		return FLT_POSTOP_FINISHED_PROCESSING;			
	}


	__try {

		// Check if the request come from the scan process.
		/*
		ntStatus = IsFromTheAnalysisProcess(FltGetRequestorProcess(Data),&bIsScanProcess);
		if (!NT_SUCCESS(ntStatus)){
			DbgPrint("[i] Error :: ArmaditoGuard!PostOperationIrpCreate :: IsFromTheAnalysisProcess failed :: status = 0x%x :: %d\n", ntStatus,ntStatus);
			__leave;
		}

		if (bIsScanProcess == TRUE) {
			DbgPrint("[-] Warning :: ArmaditoGuard!PosOperationIrpCreate :: !! Ignoring Analysis process request !! :: %wZ\n",FltObjects->FileObject->FileName);
			__leave;
		}*/

		// Ignore Directory Open Creation.
		ntStatus = FltIsDirectory(FltObjects->FileObject, FltObjects->Instance, &bIsDir);
		if (!NT_SUCCESS(Data->IoStatus.Status)) {
			DbgPrint("[-] Error :: ArmaditoGuard!PostOperationIrpCreate :: FltIsDirectory() routine failed !! \n");
			__leave;
		}

		if (bIsDir == TRUE) {
			//DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCreate :: Ignoring Directory file operation \n");
			__leave;
		}

#if 1
		// Skip encrypted files :: source avscan project.
		if (!(FlagOn(desiredAccess, FILE_WRITE_DATA)) && 
        !(FlagOn(desiredAccess, FILE_READ_DATA)) ) {
        			
			ntStatus = IsFileEncrypted( FltObjects->Instance,
										 FltObjects->FileObject,
										 &encrypted );
			if (!NT_SUCCESS( ntStatus )) {

				DbgPrint("[-] Error :: ArmaditoGuard!PostOperationIrpCreate :: AvGetFileEncrypted routine failed :: status = 0x%x\n",ntStatus);
				__leave;
			}
			if (encrypted) {
				DbgPrint("[i] Info :: ArmaditoGuard!PostOperationIrpCreate :: skip encrypted file :: [%wZ]\n",FltObjects->FileObject->FileName);
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
				DbgPrint("[-] Error :: ArmaditoGuard!PostOperationIrpCreate :: FltAllocateContext routine failed\n");
				__leave;
			}

			//DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCreate :: Context allocated :: %wZ.\n",FltObjects->FileObject->FileName);
			RtlZeroMemory(FileContext, sizeof(FILE_CONTEXT));

			// init scan result.
			FileContext->scanResult = NONE;

			ntStatus = FltSetFileContext(FltObjects->Instance,FltObjects->FileObject,FLT_SET_CONTEXT_KEEP_IF_EXISTS, FileContext, NULL );
			if (!NT_SUCCESS(ntStatus)) {
				DbgPrint("[-] Error :: ArmaditoGuard!PostOperationIrpCreate :: FltSetFileContext routine failed\n");
				__leave;
			}

			// For file creation, scan the file at cleanup.
			if (FlagOn(Data->IoStatus.Information, FILE_CREATED) ) {
				a6oDbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCreate :: [%d] :: FILE_CREATED flag :: %wZ\n", FltGetRequestorProcessId(Data),FltObjects->FileObject->FileName);
				__leave;
			}

		}

		
		// if the file has already been analyzed do no analyze it twice.
		if ( FileContext->scanResult == NONE ) {
			// Launch file analysis.
			scanRes = LaunchFileAnalysis(Data,FltObjects);
			FileContext->scanResult = scanRes;
			// Print scan result:
			if (FileContext->scanResult == ARMADITO_MALWARE || FileContext->scanResult == ARMADITO_CLEAN)
				a6oDbgPrint("[i] INFO :: ArmaditoGuard!PostOperationIrpCreate ::[%d]:: %wZ => %s\n",FltGetRequestorProcessId(Data), FltObjects->FileObject->FileName,PrintArmaditoScanResult(FileContext->scanResult));
		}
		

		/*
		if (FileContext->scanResult == ARMADITO_IERROR) {
			DbgPrint("[-] Error :: ArmaditoGuard!PostOperationIrpCreate :: LaunchFileAnalysis failed for file :: %wZ.\n",FltObjects->FileObject->FileName);	;
			__leave;
		}
		*/

		/*
		// Display filename informations.
		DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCreate :: Volume = %wZ \n",FileNameInformation->Volume);
		DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCreate :: ParentDir = %wZ \n",FileNameInformation->ParentDir);
		DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCreate :: Name = %wZ \n",FileNameInformation->Name);
		DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCreate :: FinalComponent = %wZ \n",FileNameInformation->FinalComponent);	
		DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCreate :: File created :: %wZ.\n",FltObjects->FileObject->FileName);
		*/

		// If the file is detected as a malware, cancel file operation.
		if (FileContext->scanResult == ARMADITO_MALWARE ) {

			DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCreate :: [%d] :: MALWARE DETECTED :: %wZ.\n",FltGetRequestorProcessId(Data),FltObjects->FileObject->FileName);	
		
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


_IRQL_requires_max_(APC_LEVEL)
_IRQL_requires_same_
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
				DbgPrint("[-] Error :: ArmaditoGuard!PostOperationIrpWrite :: WRITE OPERATION failed !! 0x%x :: %wZ \n",Data->IoStatus.Status,FltObjects->FileObject->FileName);
			__leave;
		}

		// Test if the requestor process is the analysis service.
		ntStatus = IsFromTheAnalysisProcess(FltGetRequestorProcess(Data),&bIsScanProcess);
		if (!NT_SUCCESS(ntStatus)){
			//DbgPrint("[i] Error :: ArmaditoGuard!PreOperationIrpCleanup :: IsFromTheAnalysisProcess failed :: status = 0x%x :: %d\n", status,status);
			__leave;
		}

		if (bIsScanProcess == TRUE) {
			//DbgPrint("[+] Debug :: ArmaditoGuard!PostOperationIrpWrite :: !! Ignoring Analysis process request !! :: %wZ\n",FltObjects->FileObject->FileName);
			__leave;
		}

		// Check IRQL Level to avoid unexpected crash.
		if (KeGetCurrentIrql( ) > APC_LEVEL) {
			DbgPrint("[-] Error :: ArmaditoGuard!PostOperationIrpWrite :: Bad IRQL level!\n");
			__leave;
		}

		// Ignore Directory Open Creation.
		ntStatus = FltIsDirectory(FltObjects->FileObject, FltObjects->Instance, &bIsDir);
		if (!NT_SUCCESS(ntStatus)) {
			DbgPrint("[-] Error :: ArmaditoGuard!PostOperationIrpWrite :: FltIsDirectory() routine failed !! 0x%x ::\n",ntStatus);
			__leave;
		}

		if (bIsDir == TRUE) {
			//DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCreate :: Ignoring Directory file operation \n");
			__leave;
		}

		ntStatus = FltGetFileContext(FltObjects->Instance, FltObjects->FileObject, (PFLT_CONTEXT *)&FileContext);

		
		if (ntStatus == STATUS_NOT_SUPPORTED) {			
			__leave;
		} else if (ntStatus == STATUS_NOT_FOUND) {

			// If there is no context found, then set a context to this file.
			ntStatus = FltAllocateContext(FltObjects->Filter, FLT_FILE_CONTEXT, sizeof(FILE_CONTEXT), NonPagedPool, (PFLT_CONTEXT)&FileContext);
			if (!NT_SUCCESS(ntStatus)) {
				DbgPrint("[-] Error :: ArmaditoGuard!PostOperationIrpWrite :: FltAllocateContext routine failed\n");
				__leave;
			}

			//DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCreate :: Context allocated :: %wZ.\n",FltObjects->FileObject->FileName);
			RtlZeroMemory(FileContext, sizeof(FILE_CONTEXT));

			// init scan result.
			FileContext->scanResult = NONE;

			ntStatus = FltSetFileContext(FltObjects->Instance,FltObjects->FileObject,FLT_SET_CONTEXT_KEEP_IF_EXISTS, FileContext, NULL );
			if (!NT_SUCCESS(ntStatus)) {
				DbgPrint("[-] Error :: ArmaditoGuard!PostOperationIrpWrite :: FltSetFileContext routine failed\n");
				__leave;
			}

		}
		else if (NT_SUCCESS(ntStatus)) {	// If a context is already set for this file.
			
			if (FileContext->scanResult == ARMADITO_CLEAN)
				//DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpWrite :: Writing in MARKED file %s :: %wZ\n",PrintArmaditoScanResult(FileContext->scanResult),FltObjects->FileObject->FileName);
			
			// If the file was no detected as a malware, then rescan it later.
			if (FileContext->scanResult != ARMADITO_MALWARE) {
				FileContext->scanResult = NONE; // Reset the scan result.
			}
			else {
				// Cancel operation. in the postoperation.
				// TODO;
				DbgPrint("[+] Debug :: ArmaditoGuard!PostOperationIrpWrite :: Writing in ARMADITO_MALWARE marked file :: %wZ\n",FltObjects->FileObject->FileName);
				__leave;

			}
			
			__leave; //return FLT_POSTOP_FINISHED_PROCESSING;

		}
		
		
		//FltReleaseContext((PFLT_CONTEXT)FileContext);
		//DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCleanup :: ::Context realeased for file %wZ\n",FltObjects->FileObject->FileName);


	}
	__finally {

		if (FileContext != NULL) {
			//DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpWrite :: Freeing context in Write callback :: %wZ\n",FltObjects->FileObject->FileName);
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
			//DbgPrint("[i] Error :: ArmaditoGuard!PreOperationIrpCleanup :: IsFromTheAnalysisProcess failed :: status = 0x%x :: %d\n", status,status);
			__leave;
		}

		if (bIsScanProcess == TRUE) {
			//DbgPrint("[+] Debug :: ArmaditoGuard!PreOperationIrpCleanup :: !! Ignoring Analysis process request !! :: %wZ\n",FltObjects->FileObject->FileName);
			__leave;
		}

		// Ignore directories
		ntStatus = FltIsDirectory(FltObjects->FileObject, FltObjects->Instance, &bIsDirectory);
		if (!NT_SUCCESS(Data->IoStatus.Status)) {
			DbgPrint("[-] Error :: ArmaditoGuard!PostOperationIrpCleanup :: FltIsDirectory() routine failed !! \n");
			__leave;
		}

		if (bIsDirectory == TRUE)
			__leave;

		if(FlagOn(FltObjects->FileObject->Flags, FO_VOLUME_OPEN)){
			__leave;
		}

				

		
		//DbgPrint("[+] Debug :: ArmaditoGuard!PreOperationIrpCleanup :: %wZ\n", FltObjects->FileObject->FileName);
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
		if (FileContext->scanResult != NONE && FileContext->scanResult != TIMEOUT && FileContext->scanResult != ARMADITO_UNKNOWN_FILE_TYPE ) {
			//DbgPrint("[+] Debug :: ArmaditoGuard!PreOperationIrpCleanup :: File Already Scanned  :: %wZ :: %s\n", FltObjects->FileObject->FileName, PrintArmaditoScanResult(FileContext->scanResult));
			__leave;
		}
			

		// Scan the file through the analysis service.
		scanResult = LaunchFileAnalysis(Data,FltObjects);
		FileContext->scanResult = scanResult;

		//if (FileContext->scanResult == ARMADITO_MALWARE || FileContext->scanResult == ARMADITO_CLEAN)
		a6oDbgPrint("[i] INFO :: ArmaditoGuard!PreOperationIrpCleanup ::[%d]:: %wZ => %s\n",FltGetRequestorProcessId(Data),FltObjects->FileObject->FileName,PrintArmaditoScanResult(FileContext->scanResult));


		// if the file has been detected has malware...
		if (FileContext->scanResult == ARMADITO_MALWARE) {
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
	UNREFERENCED_PARAMETER( Data ); 
    UNREFERENCED_PARAMETER( CompletionContext );

	__try {

		ntStatus = FltGetFileContext(FltObjects->Instance, FltObjects->FileObject, (PFLT_CONTEXT *)&FileContext);

		if (!NT_SUCCESS(ntStatus)) {			
			__leave; //return FLT_POSTOP_FINISHED_PROCESSING;
		}

		a6oDbgPrint("[+] Debug :: ArmaditoGuard!PostOperationIrpCleanup :: Cleaning File Context :: %wZ => %s\n", FltObjects->FileObject->FileName,PrintArmaditoScanResult(FileContext->scanResult));
		
		
		//DbgPrint("[i] Debug :: ArmaditoGuard!PostOperationIrpCleanup :: ::Context realeased for file %wZ\n",FltObjects->FileObject->FileName);

		// maybe use FltDeleteFileContext( ) instead of FltDeleteContext();		
			
		if (FileContext != NULL && FileContext->scanResult != ARMADITO_MALWARE ) {
			FltReleaseContext((PFLT_CONTEXT)FileContext);
			FltDeleteContext((PFLT_CONTEXT)FileContext);
			FileContext = NULL;
		}

	}
	__finally {

	}

    return retstatus;
}
