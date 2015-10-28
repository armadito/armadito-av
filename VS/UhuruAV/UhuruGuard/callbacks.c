#include "callbacks.h"
#include "Struct.h"
#include "communication.h"


// This function launch a Scan with the analysis process.
// This function is a FAKE scan.
SCAN_RESULT LaunchFileAnalysis(_In_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects) {

	NTSTATUS ntStatus;
	SCAN_RESULT res = NONE;
	PFLT_FILE_NAME_INFORMATION FileNameInformation = NULL;	
	UNICODE_STRING malName;
	SCAN_RESULT answer = NONE;
	
	
	UNREFERENCED_PARAMETER( FltObjects );

#if 1

	__try {

		
		// For test purposes.	
		RtlInitUnicodeString(&malName,L"malware.txt");
		

		// Retrieve file name informations.
		ntStatus = FltGetFileNameInformation(Data,FLT_FILE_NAME_NORMALIZED|FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInformation);
		if (!NT_SUCCESS(ntStatus)) {
			DbgPrint("[-] Error :: UhuruGuard!LaunchFileAnalysis :: FltGetFileNameInformation() routine failed :: 0x%x \n",ntStatus);
			res = ERROR;
			__leave;			
		}

		ntStatus = FltParseFileNameInformation(FileNameInformation);
		if (!NT_SUCCESS(ntStatus)) {
			
			DbgPrint("[-] Error :: UhuruGuard!LaunchFileAnalysis :: FltParseFileNameInformation() routine failed :: 0x%x \n", ntStatus);
			res = ERROR;
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
			res = ERROR;
			__leave;			
		}

		//-----------------------------------------

		if ( RtlEqualUnicodeString(&FileNameInformation->FinalComponent,&malName,FALSE) == TRUE ) {	
			res = MALWARE;		
		}
		else {
			res = CLEAN;
		}

	}
	__finally {

		if (FileNameInformation != NULL) {
			FltReleaseFileNameInformation(FileNameInformation);
			FileNameInformation = NULL;
		}

		/*
		if (&malName != NULL) {
			RtlFreeUnicodeString(&malName);
		}*/

	}
	

#endif

	return res;
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
  

	//UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );


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
    NTSTATUS ntStatus, retStatus  = FLT_POSTOP_FINISHED_PROCESSING;
	BOOLEAN bIsDir = FALSE;	
	//BOOLEAN bleave = FALSE;
	PFILE_CONTEXT FileContext = NULL;
	SCAN_RESULT scanRes;

	UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( CompletionContext );


	__try {


		// Check if the operation succeeded or not.
		if (!NT_SUCCESS(Data->IoStatus.Status)) {
			//DbgPrint("[-] Warning :: UhuruGuard!PostOperationIrpCreate :: Irp create Operation failed before treatment.\n");
			__leave;
		}
   
		// Ignore Volume Requests.
		if (FlagOn(FltObjects->FileObject->Flags, FO_VOLUME_OPEN)) {
			DbgPrint("[-] Warning :: UhuruGuard!PostOperationIrpCreate :: Ignore VOLUME_OPEN requests.\n");			
			__leave;			
		}

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
			//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: Context already set for the file %wZ\n",FltObjects->FileObject->FileName);
			__leave;
		}

		// If there is no context set for this file.
		if (ntStatus == STATUS_NOT_FOUND) {

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
				DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: FILE_CREATED flag :: %wZ\n", FltObjects->FileObject->FileName);
				return FLT_POSTOP_FINISHED_PROCESSING;
			}

		}

		// Launch file analysis.
		scanRes = LaunchFileAnalysis(Data,FltObjects);
		FileContext->scanResult = scanRes;

		if (FileContext->scanResult == ERROR) {
			DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpCreate :: LaunchFileAnalysis failed for file :: %wZ.\n",FltObjects->FileObject->FileName);	;
			__leave;
		}

		/*
		// Display filename informations.
		DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: Volume = %wZ \n",FileNameInformation->Volume);
		DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: ParentDir = %wZ \n",FileNameInformation->ParentDir);
		DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: Name = %wZ \n",FileNameInformation->Name);
		DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: FinalComponent = %wZ \n",FileNameInformation->FinalComponent);	
		DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: File created :: %wZ.\n",FltObjects->FileObject->FileName);
		*/

		// If the file is detected as a malware, cancel file operation.
		if (FileContext->scanResult == MALWARE ) {

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

		if (FileContext != NULL) {
			FltReleaseContext((PFLT_CONTEXT)FileContext);
			FileContext = NULL;
		}
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
	//HANDLE fhdle = NULL;
	//int flag = 0;
	PFILE_CONTEXT FileContext = NULL;
	BOOLEAN bIsDir = FALSE;

	//UNREFERENCED_PARAMETER( Data );
	UNREFERENCED_PARAMETER( Flags );
    //UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

	__try {

		// If the size the content to write is NULL, then leave.
		if (Data->Iopb->Parameters.Write.Length == 0) {
			__leave;
		}

		// Ignore Directory Open Creation.
		ntStatus = FltIsDirectory(FltObjects->FileObject, FltObjects->Instance, &bIsDir);
		if (!NT_SUCCESS(Data->IoStatus.Status)) {
			DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpWrite :: FltIsDirectory() routine failed !! 0x%x \n",ntStatus);
			__leave;
		}

		if (bIsDir == TRUE) {
			//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: Ignoring Directory file operation \n");
			__leave;
		}

		ntStatus = FltGetFileContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, (PFLT_CONTEXT *)&FileContext);

		
		if (ntStatus == STATUS_NOT_SUPPORTED) {			
			__leave;
		}

		// If there is no context found, then set a context to this file.
		if (ntStatus == STATUS_NOT_FOUND) {

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

		if (!NT_SUCCESS(ntStatus)) {
			
			// If the file was clean, then rescan it later.
			if (FileContext->scanResult != MALWARE) {
				FileContext->scanResult = NONE; // Reset the scan result.
			}
			else {
				// Cancel operation. in the postoperation.
				// TODO;
				DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpWrite :: Writing in MALWARE marked file :: %wZ\n",FltObjects->FileObject->FileName);
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
			FileContext = NULL;
		}

	}


    return retstatus;
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
	UNREFERENCED_PARAMETER( FltObjects ); 
    UNREFERENCED_PARAMETER( CompletionContext );

	__try {

		ntStatus = FltGetFileContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, (PFLT_CONTEXT *)&FileContext);

		if (!NT_SUCCESS(ntStatus)) {
			//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCreate :: Context already set for the file %wZ\n",FltObjects->FileObject->FileName);
			__leave; //return FLT_POSTOP_FINISHED_PROCESSING;
		}
		
		// In this case the file has not been analyzed.
		/*
		if (FileContext->scanResult == NONE) {
			
			//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCleanup :: Context realeased for new file :: %wZ\n",FltObjects->FileObject->FileName);			
			// Scan the file.
			scanRes = LaunchFileAnalysis(Data, FltObjects);

			if (scanRes == ERROR) {
				DbgPrint("[-] Error :: UhuruGuard!PostOperationIrpCleanup :: LaunchFileAnalysis failed for file :: %wZ.\n",FltObjects->FileObject->FileName);				
			}
			
		}
		*/
		
		FltReleaseContext((PFLT_CONTEXT)FileContext);
		//DbgPrint("[i] Debug :: UhuruGuard!PostOperationIrpCleanup :: ::Context realeased for file %wZ\n",FltObjects->FileObject->FileName);

		// maybe use FltDeleteFileContext( ) instead of FltDeleteContext() ;
		FltDeleteContext((PFLT_CONTEXT)FileContext);
		FileContext = NULL;

	}
	__finally {

	}

    return retstatus;
}