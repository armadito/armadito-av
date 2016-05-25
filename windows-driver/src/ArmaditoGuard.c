/*++

Module Name:

    ArmaditoGuard.c

Abstract:

    This is the main module of the ArmaditoGuard miniFilter driver.

Environment:

    Kernel mode

--*/

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include "callbacks.h"
#include "communication.h"
#include "ArmaditoGuard.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")


PFLT_FILTER gFilterHandle;
PFLT_PORT gFilterComPort = NULL;
ULONG_PTR OperationStatusCtx = 1;

#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

//# TAGs
#define FLT_CONTEXT_FLAG 'FxtC'

ULONG gTraceFlags = 0;


#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(gTraceFlags,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

/*************************************************************************
    Prototypes
*************************************************************************/

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

VOID CleanupFileContext(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType);

NTSTATUS
ArmaditoGuardInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID
ArmaditoGuardInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID
ArmaditoGuardInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS
ArmaditoGuardUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS
ArmaditoGuardInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
ArmaditoGuardPreOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

VOID
ArmaditoGuardOperationStatusCallback (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    _In_ NTSTATUS OperationStatus,
    _In_ PVOID RequesterContext
    );

FLT_POSTOP_CALLBACK_STATUS
ArmaditoGuardPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
ArmaditoGuardPreOperationNoPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

BOOLEAN
ArmaditoGuardDoRequestOperationStatus(
    _In_ PFLT_CALLBACK_DATA Data
    );

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, ArmaditoGuardUnload)
#pragma alloc_text(PAGE, ArmaditoGuardInstanceQueryTeardown)
#pragma alloc_text(PAGE, ArmaditoGuardInstanceSetup)
#pragma alloc_text(PAGE, ArmaditoGuardInstanceTeardownStart)
#pragma alloc_text(PAGE, ArmaditoGuardInstanceTeardownComplete)
#endif

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {

	{ IRP_MJ_CREATE,
      0,
      PreOperationIrpCreate,
      PostOperationIrpCreate },

	{ IRP_MJ_WRITE,
      0,
      NULL,
      PostOperationIrpWrite },

	{ IRP_MJ_CLEANUP,
      0,
      PreOperationIrpCleanup,
      PostOperationIrpCleanup },

#if 0 // TODO - List all of the requests to filter.
    { IRP_MJ_CREATE,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_CREATE_NAMED_PIPE,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_CLOSE,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_READ,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_WRITE,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_QUERY_INFORMATION,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_SET_INFORMATION,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_QUERY_EA,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_SET_EA,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_FLUSH_BUFFERS,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_QUERY_VOLUME_INFORMATION,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_SET_VOLUME_INFORMATION,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_DIRECTORY_CONTROL,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_FILE_SYSTEM_CONTROL,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_DEVICE_CONTROL,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_INTERNAL_DEVICE_CONTROL,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_SHUTDOWN,
      0,
      ArmaditoGuardPreOperationNoPostOperation,
      NULL },                               //post operations not supported

    { IRP_MJ_LOCK_CONTROL,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_CLEANUP,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_CREATE_MAILSLOT,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_QUERY_SECURITY,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_SET_SECURITY,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_QUERY_QUOTA,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_SET_QUOTA,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_PNP,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_ACQUIRE_FOR_MOD_WRITE,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_RELEASE_FOR_MOD_WRITE,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_ACQUIRE_FOR_CC_FLUSH,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_RELEASE_FOR_CC_FLUSH,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_NETWORK_QUERY_OPEN,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_MDL_READ,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_MDL_READ_COMPLETE,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_PREPARE_MDL_WRITE,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_MDL_WRITE_COMPLETE,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_VOLUME_MOUNT,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

    { IRP_MJ_VOLUME_DISMOUNT,
      0,
      ArmaditoGuardPreOperation,
      ArmaditoGuardPostOperation },

#endif // TODO

    { IRP_MJ_OPERATION_END }
};


//
//  Context registration
//
CONST FLT_CONTEXT_REGISTRATION ContextRegistration[] = {

	{	FLT_FILE_CONTEXT,
		0,
		NULL,//(PFLT_CONTEXT_CLEANUP_CALLBACK)CleanupFileContext,
		sizeof(FILE_CONTEXT),
		FLT_CONTEXT_FLAG
	},
	{ FLT_CONTEXT_END }
};



//
//  This defines what we want to filter with FltMgr
//

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags
    ContextRegistration,                               //  Context
    Callbacks,                          //  Operation callbacks

    ArmaditoGuardUnload,                           //  MiniFilterUnload

    ArmaditoGuardInstanceSetup,                    //  InstanceSetup
    ArmaditoGuardInstanceQueryTeardown,            //  InstanceQueryTeardown
    ArmaditoGuardInstanceTeardownStart,            //  InstanceTeardownStart
    ArmaditoGuardInstanceTeardownComplete,         //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};

/*++

Routine Description:

    The given context is being freed.
    Free the allocated name buffer if there one.

Arguments:

    Context - The context being freed

    ContextType - The type of context this is

Return Value:

    None

--*/
VOID CleanupFileContext(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType){
    //PFILE_CONTEXT ctx = NULL;
	//NTSTATUS ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( ContextType );
	UNREFERENCED_PARAMETER( Context );

	DbgPrint("[-] Debug :: ArmaditoGuard!CleanupFileContext :: CleanupFileContext!!\n");

	//FLT_ASSERT(ContextType == FLT_FILE_CONTEXT);

	//ctx = (PFILE_CONTEXT)Context;
	//DebugPrint(TRACE_LEVEL_INFORMATION, FLT_CONTEXT_FILE, "[+] (FileCtx) Free the context : 0x%p | ContextType : %#I64x.", ctx, ContextType);
	//}
	//RtlZeroMemory(ctx, sizeof(FILE_CONTEXT));

}



NTSTATUS
ArmaditoGuardInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    )
/*++

Routine Description:

    This routine is called whenever a new instance is created on a volume. This
    gives us a chance to decide if we need to attach to this volume or not.

    If this routine is not defined in the registration structure, automatic
    instances are always created.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Flags describing the reason for this attach request.

Return Value:

    STATUS_SUCCESS - attach
    STATUS_FLT_DO_NOT_ATTACH - do not attach

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("ArmaditoGuard!ArmaditoGuardInstanceSetup: Entered\n") );

	DbgPrint("[i] Debug :: ArmaditoGuard!ArmaditoGuardInstanceSetup :: New Volume mounted\n");

    return STATUS_SUCCESS;
}


NTSTATUS
ArmaditoGuardInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This is called when an instance is being manually deleted by a
    call to FltDetachVolume or FilterDetach thereby giving us a
    chance to fail that detach request.

    If this routine is not defined in the registration structure, explicit
    detach requests via FltDetachVolume or FilterDetach will always be
    failed.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Indicating where this detach request came from.

Return Value:

    Returns the status of this operation.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("ArmaditoGuard!ArmaditoGuardInstanceQueryTeardown: Entered\n") );

    return STATUS_SUCCESS;
}


VOID
ArmaditoGuardInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This routine is called at the start of instance teardown.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Reason why this instance is being deleted.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("ArmaditoGuard!ArmaditoGuardInstanceTeardownStart: Entered\n") );
}


VOID
ArmaditoGuardInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This routine is called at the end of instance teardown.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Reason why this instance is being deleted.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("ArmaditoGuard!ArmaditoGuardInstanceTeardownComplete: Entered\n") );
}







/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This is the initialization routine for this miniFilter driver.  This
    registers with FltMgr and initializes all global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.

    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Routine can return non success error codes.

--*/
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER( RegistryPath );

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("ArmaditoGuard!DriverEntry: Entered\n") );

    //
    //  Register with FltMgr to tell it our callback routines
    //
    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &gFilterHandle );

    FLT_ASSERT( NT_SUCCESS( status ) );

    if (NT_SUCCESS( status )) {

		// Init communication
		status = InitCommunicationPort(gFilterHandle, &gFilterComPort);

		if (!NT_SUCCESS(status)) {
			FltUnregisterFilter( gFilterHandle );
			return status;
		}

        //
        //  Start filtering i/o
        //
		DbgPrint("[+] Debug :: ArmaditoGuard!DriverEntry :: FltRegisterFilter succeeded !\n");
        status = FltStartFiltering( gFilterHandle );

        if (!NT_SUCCESS( status )) {
			DbgPrint("[-] Error :: ArmaditoGuard!DriverEntry :: FltStartFiltering() failed!\n");

			if (gFilterComPort != NULL) {
				 FltCloseCommunicationPort( gFilterComPort );
			}

            FltUnregisterFilter( gFilterHandle );
		
		} else 
			DbgPrint("[+] Debug :: ArmaditoGuard!DriverEntry :: FltStartFiltering succeeded \n");

		/*if (gFilterComPort != NULL) {
			 FltCloseCommunicationPort( gFilterComPort );
		}*/
    }

    return status;
}

NTSTATUS
ArmaditoGuardUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
/*++

Routine Description:

    This is the unload routine for this miniFilter driver. This is called
    when the minifilter is about to be unloaded. We can fail this unload
    request if this is not a mandatory unload indicated by the Flags
    parameter.

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    Returns STATUS_SUCCESS.

--*/
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    //PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
     //             ("ArmaditoGuard!ArmaditoGuardUnload: Entered\n") );

	if (gFilterComPort != NULL) {
		 FltCloseCommunicationPort( gFilterComPort );
	}
	DbgPrint("[+] Debug :: ArmaditoGuard!ArmaditoGuardUnload :: communication port closed successfully !\n");

    FltUnregisterFilter( gFilterHandle );

	DbgPrint("[+] Debug :: ArmaditoGuard!ArmaditoGuardUnload :: Driver unloaded successfully !\n");


    return STATUS_SUCCESS;
}


/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/
FLT_PREOP_CALLBACK_STATUS
ArmaditoGuardPreOperation (
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
    NTSTATUS status;

    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("ArmaditoGuard!ArmaditoGuardPreOperation: Entered\n") );

    //
    //  See if this is an operation we would like the operation status
    //  for.  If so request it.
    //
    //  NOTE: most filters do NOT need to do this.  You only need to make
    //        this call if, for example, you need to know if the oplock was
    //        actually granted.
    //

    if (ArmaditoGuardDoRequestOperationStatus( Data )) {

        status = FltRequestOperationStatusCallback( Data,
                                                    ArmaditoGuardOperationStatusCallback,
                                                    (PVOID)(++OperationStatusCtx) );
        if (!NT_SUCCESS(status)) {

            PT_DBG_PRINT( PTDBG_TRACE_OPERATION_STATUS,
                          ("ArmaditoGuard!ArmaditoGuardPreOperation: FltRequestOperationStatusCallback Failed, status=%08x\n",
                           status) );
        }
    }

    // This template code does not do anything with the callbackData, but
    // rather returns FLT_PREOP_SUCCESS_WITH_CALLBACK.
    // This passes the request down to the next miniFilter in the chain.

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}



VOID
ArmaditoGuardOperationStatusCallback (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    _In_ NTSTATUS OperationStatus,
    _In_ PVOID RequesterContext
    )
/*++

Routine Description:

    This routine is called when the given operation returns from the call
    to IoCallDriver.  This is useful for operations where STATUS_PENDING
    means the operation was successfully queued.  This is useful for OpLocks
    and directory change notification operations.

    This callback is called in the context of the originating thread and will
    never be called at DPC level.  The file object has been correctly
    referenced so that you can access it.  It will be automatically
    dereferenced upon return.

    This is non-pageable because it could be called on the paging path

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    RequesterContext - The context for the completion routine for this
        operation.

    OperationStatus -

Return Value:

    The return value is the status of the operation.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("ArmaditoGuard!ArmaditoGuardOperationStatusCallback: Entered\n") );

    PT_DBG_PRINT( PTDBG_TRACE_OPERATION_STATUS,
                  ("ArmaditoGuard!ArmaditoGuardOperationStatusCallback: Status=%08x ctx=%p IrpMj=%02x.%02x \"%s\"\n",
                   OperationStatus,
                   RequesterContext,
                   ParameterSnapshot->MajorFunction,
                   ParameterSnapshot->MinorFunction,
                   FltGetIrpName(ParameterSnapshot->MajorFunction)) );
}


FLT_POSTOP_CALLBACK_STATUS
ArmaditoGuardPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    )
/*++

Routine Description:

    This routine is the post-operation completion routine for this
    miniFilter.

    This is non-pageable because it may be called at DPC level.

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The completion context set in the pre-operation routine.

    Flags - Denotes whether the completion is successful or is being drained.

Return Value:

    The return value is the status of the operation.

--*/
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("ArmaditoGuard!ArmaditoGuardPostOperation: Entered\n") );

    return FLT_POSTOP_FINISHED_PROCESSING;
}


FLT_PREOP_CALLBACK_STATUS
ArmaditoGuardPreOperationNoPostOperation (
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
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("ArmaditoGuard!ArmaditoGuardPreOperationNoPostOperation: Entered\n") );

    // This template code does not do anything with the callbackData, but
    // rather returns FLT_PREOP_SUCCESS_NO_CALLBACK.
    // This passes the request down to the next miniFilter in the chain.

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}


BOOLEAN
ArmaditoGuardDoRequestOperationStatus(
    _In_ PFLT_CALLBACK_DATA Data
    )
/*++

Routine Description:

    This identifies those operations we want the operation status for.  These
    are typically operations that return STATUS_PENDING as a normal completion
    status.

Arguments:

Return Value:

    TRUE - If we want the operation status
    FALSE - If we don't

--*/
{
    PFLT_IO_PARAMETER_BLOCK iopb = Data->Iopb;

    //
    //  return boolean state based on which operations we are interested in
    //

    return (BOOLEAN)

            //
            //  Check for oplock operations
            //

             (((iopb->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
               ((iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_FILTER_OPLOCK)  ||
                (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_BATCH_OPLOCK)   ||
                (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_1) ||
                (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_2)))

              ||

              //
              //    Check for directy change notification
              //

              ((iopb->MajorFunction == IRP_MJ_DIRECTORY_CONTROL) &&
               (iopb->MinorFunction == IRP_MN_NOTIFY_CHANGE_DIRECTORY))
             );
}
