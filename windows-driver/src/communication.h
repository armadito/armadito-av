#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include "structs.h"

#define MAX_PATH_SIZE 512
#define MESSAGE_CONTEXT_BUF 'bgsm'


typedef struct _COMMUNICATION_PORT_CONTEXT {
	

	//
	//	Identification code of the file.
	//
	//IDENTIFICATION_MARK_FILES IdentificationFileType;

	//
	//	Type of the device where the file stands.
	//
	DEVICE_TYPE DeviceType;

	//
	//	Command given to the scan process to know what to do.
	//
	//COMMUNICATION_COMMAND Command;

	//
	//	Length of the file name.
	//
	ULONG FileNameLength;
	
	//
	//	File name buffer. This one should be enough long
	//	for almost of the normal cases. Note that, a handle
	//	to the file could be shared with user-mode rights from
	//	the kernel (case of AvScan project from M$).
	//
	CHAR FileName[MAX_PATH_SIZE];

	// Scan result Only used in the reply message.
	SCAN_RESULT scan_result;


} MESSAGE_CONTEXT, *PMESSAGE_CONTEXT; // Change to SCAN_MESSAGE_CONTEXT

NTSTATUS InitCommunicationPort(_In_ PFLT_FILTER FilterHandle, _Out_ PFLT_PORT *ServerPort);

NTSTATUS ConnectNotifyCallback(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_ PVOID ConnectionContext,
	IN ULONG SizeOfContext,
	OUT PVOID *ConnectionPortCookie);

VOID DisconnectNotifyCallback(IN PVOID ConnectionCookie);

NTSTATUS
MessageNotifyCallback(
IN PVOID PortCookie,
IN PVOID InputBuffer OPTIONAL,
IN ULONG InputBufferLength,
OUT PVOID OutputBuffer OPTIONAL,
IN ULONG OutputBufferLength,
OUT PULONG ReturnOutputBufferLength
);

//NTSTATUS SendScanOrder(_In_ PFLT_FILTER FilterHandle, _In_ PFLT_PORT ClientPort, PUNICODE_STRING FilePath, _Out_ SCAN_RESULT * AnswerMessage);
NTSTATUS SendScanOrder(_In_ PFLT_FILTER FilterHandle, PUNICODE_STRING FilePath, _Out_ SCAN_RESULT * AnswerMessage);
NTSTATUS IsFromTheAnalysisProcess(PEPROCESS Process, PBOOLEAN Answer);

#endif