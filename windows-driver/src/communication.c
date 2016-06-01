/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito windows driver.

Reproduction, distribution and derivative works are permitted under the terms of the Microsoft Public License
See file COPYING.MSPL for terms of license.

***/

#include "communication.h"

#define SCAN_PORT_NAME L"\\A6oPortScanFilter"
/***********************************************************************
	InitCommunicationPort(gFilterHandle, &gFilterComPort)
************************************************************************/

extern PFLT_FILTER gFilterHandle;
PFLT_PORT gClientComPort = NULL;
PEPROCESS gScanProcess = NULL;
//HANDLE gScanProcessId = NULL;


NTSTATUS IsFromTheAnalysisProcess(PEPROCESS Process , PBOOLEAN Answer) {

	NTSTATUS ntStatus = STATUS_SUCCESS;

	//	Check parameters
	if(Process == NULL){
		return STATUS_INVALID_PARAMETER_1;
	}
	if(Answer == NULL){
		return STATUS_INVALID_PARAMETER_2;
	}

	// Initialize the answer.
	*Answer = FALSE;

	if (gClientComPort == NULL) {
		ntStatus = STATUS_CONNECTION_ABORTED;
		return ntStatus;
	}
	if (gScanProcess == NULL) {
		ntStatus = STATUS_CONNECTION_ABORTED;
		return ntStatus;
	}

	

	//PsGetProcessId

	// psGetCurrent
	if (Process == gScanProcess) {
		*Answer = TRUE;
	}

	return ntStatus;
}



NTSTATUS InitCommunicationPort(_In_ PFLT_FILTER FilterHandle, _Out_ PFLT_PORT *ServerPort){

	NTSTATUS ntStatus = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES ObjectAttributes;
	UNICODE_STRING PortName;// = RTL_CONSTANT_STRING(L"\\ArmaditoPortScanFilter");
	PFLT_PORT LocalServerPort = NULL;
	PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
	// Create Security Descriptor/

	//UNREFERENCED_PARAMETER( FilterHandle);
	//UNREFERENCED_PARAMETER(LocalServerPort );
	//UNREFERENCED_PARAMETER( ServerPort);
	//UNREFERENCED_PARAMETER( ObjectAttributes);
	//UNREFERENCED_PARAMETER( PortName);

	__try {

		//Builds a default security descriptor for use with FltCreateCommunicationPort.
		ntStatus = FltBuildDefaultSecurityDescriptor(&SecurityDescriptor, FLT_PORT_ALL_ACCESS);
		if (!NT_SUCCESS(ntStatus)) {
			DbgPrint("[-] Error :: ArmaditoGuard!InitCommunicationPort :: FltBuildDefaultSecurityDescriptor() routine failed !! \n");
			__leave;
		}

		ntStatus = RtlSetDaclSecurityDescriptor(SecurityDescriptor, TRUE, NULL, FALSE);
		if(!NT_SUCCESS(ntStatus)){
			DbgPrint("[-] Error :: ArmaditoGuard!InitCommunicationPort :: RtlSetDaclSecurityDescriptor() routine failed !! \n");
			__leave;
		}
		
		RtlInitUnicodeString(&PortName, SCAN_PORT_NAME);

		// Initialize Object Atrtibutes.
		InitializeObjectAttributes(&ObjectAttributes, &PortName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE ,NULL,SecurityDescriptor);

		
		ntStatus = FltCreateCommunicationPort(	FilterHandle,								// Filter handle
												&LocalServerPort,							// Server port
												&ObjectAttributes,							// ObjectAttributes
												NULL,										// ServerPortCookie.
												(PFLT_CONNECT_NOTIFY)ConnectNotifyCallback,			// Minifilter cals this routine whenever a user-mode apps calls FilterConnectCommunicationPort.
												(PFLT_DISCONNECT_NOTIFY)DisconnectNotifyCallback,		// Minifilter call this routine whenever the user-mode handle count for the client port reached 0; or the minifilter driver is about to unload.
												(PFLT_MESSAGE_NOTIFY)MessageNotifyCallback,	// Minifilter call this routine whenever a user-mode app calls FilterSendMessage to send a message.
												1);	// MaxConnections
	

		if (!NT_SUCCESS(ntStatus)) {
			DbgPrint("[-] Error :: ArmaditoGuard!InitCommunicationPort :: FltCreateCommunicationPort() routine failed !! \n");
			__leave;
		}

		

		// TEST
		//FltCloseCommunicationPort(LocalServerPort);

		(*ServerPort) = LocalServerPort;

		DbgPrint("[+] Debug :: ArmaditoGuard!InitCommunicationPort :: FltCreateCommunicationPort() has succeded :: serverPort = 0x%p. \n",(PVOID)(*ServerPort));
		
		/*
		if(SecurityDescriptor != NULL){
			FltFreeSecurityDescriptor(SecurityDescriptor);
		}		
		/*
		
		DbgPrint("[+] Debug :: ArmaditoGuard!InitCommunicationPort :: FltCreateCommunicationPort() has succeded :: serverPort = 0x%p. \n",(PVOID)(*ServerPort));
		*/


	}
	__finally {

		//	Free the security descriptor previously allocated if necessary.
		//
		if(SecurityDescriptor != NULL){
			FltFreeSecurityDescriptor(SecurityDescriptor);
			DbgPrint("[+] Debug :: ArmaditoGuard!InitCommunicationPort :: Free SecurityDescriptor !!\n");
		}

	}

	return ntStatus;
}

// This routine is called at IRQL = PASSIVE_LEVEL
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS ConnectNotifyCallback(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_ PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Out_ PVOID *ConnectionPortCookie) {

	UNREFERENCED_PARAMETER( ConnectionPortCookie);
	UNREFERENCED_PARAMETER(ServerPortCookie );
	UNREFERENCED_PARAMETER( ClientPort);
	

	if (ConnectionContext == NULL || SizeOfContext == 0) {
		return STATUS_INVALID_PARAMETER;
	}

	DbgPrint("[+] Debug :: ArmaditoGuard!ConnectNotifyCallback :: Client connected to the Driver !!\n");


	// init global varible for the Client Port handle.
	gClientComPort = ClientPort;
	gScanProcess = PsGetCurrentProcess();
	//gScanProcessId = PsGetCurrentProcessId;

	return STATUS_SUCCESS;

}


VOID DisconnectNotifyCallback(IN PVOID ConnectionCookie) {

	UNREFERENCED_PARAMETER(ConnectionCookie );
	DbgPrint("[+] Debug :: ArmaditoGuard!DisconnectNotifyCallback :: Callback DisconnectNotifyCallback !! \n");

	if (gClientComPort != NULL && gFilterHandle != NULL) {
		FltCloseClientPort(gFilterHandle,&gClientComPort);
		gClientComPort = NULL;
		gScanProcess = NULL;
	}


	return;
}

NTSTATUS
MessageNotifyCallback(
IN PVOID PortCookie,
IN PVOID InputBuffer OPTIONAL,
IN ULONG InputBufferLength,
OUT PVOID OutputBuffer OPTIONAL,
IN ULONG OutputBufferLength,
OUT PULONG ReturnOutputBufferLength
) {

	UNREFERENCED_PARAMETER(PortCookie );
	UNREFERENCED_PARAMETER(InputBuffer );
	UNREFERENCED_PARAMETER(InputBufferLength );
	UNREFERENCED_PARAMETER( OutputBuffer);
	UNREFERENCED_PARAMETER(OutputBufferLength );
	UNREFERENCED_PARAMETER(ReturnOutputBufferLength );
	
	DbgPrint("[-] Debug :: ArmaditoGuard!MessageNotifyCallback :: Message received from user-mode app!! \n");


	return STATUS_SUCCESS;
}


NTSTATUS SendScanOrder( _In_ PFLT_FILTER FilterHandle, PUNICODE_STRING FilePath ,  _Out_ SCAN_RESULT * AnswerMessage) {

	NTSTATUS ntStatus = STATUS_SUCCESS;
	SCAN_RESULT replyMessage = NONE;
	PMESSAGE_CONTEXT scanMessage = NULL;
	LARGE_INTEGER liTimeOut;
	LARGE_INTEGER timeOut = {0};
	ANSI_STRING AnsiString;
	ULONG replyLength =0;
	LONGLONG _1ms = 10000;
	LONGLONG localScanTimeout = 1000;



	RtlZeroMemory(&AnsiString, sizeof(ANSI_STRING));
	
	// Check parameters.
	if (FilterHandle == NULL) {
		DbgPrint("[-] Error :: ArmaditoGuard!SendScanOrder :: Invalid filter handle parameter !! \n");
		return STATUS_INVALID_PARAMETER;
	}

	if (gClientComPort == NULL) {
		DbgPrint("[-] Error :: ArmaditoGuard!SendScanOrder :: Scan service not connected yet !! \n");
		return STATUS_INVALID_PARAMETER;
	}

	if (FilePath == NULL) {
		DbgPrint("[-] Error :: ArmaditoGuard!SendScanOrder :: Invalid FilePath parameter !! \n");
		return STATUS_INVALID_PARAMETER;
	}

	if (AnswerMessage == NULL) {
		DbgPrint("[-] Error :: ArmaditoGuard!SendScanOrder :: Invalid AnswerMessage parameter !! \n");
		return STATUS_INVALID_PARAMETER;
	}


	__try {

		// Allocate memory and initialize the scan struct.
		scanMessage = (PMESSAGE_CONTEXT)ExAllocatePoolWithTag(NonPagedPool, sizeof(MESSAGE_CONTEXT), MESSAGE_CONTEXT_BUF);
		if (scanMessage == NULL) {
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			__leave;
		}
		RtlZeroMemory(scanMessage, sizeof(MESSAGE_CONTEXT));

		// Convert unicode string to ansi string for ring 3 process.		
		ntStatus = RtlUnicodeStringToAnsiString(&AnsiString, (PCUNICODE_STRING)FilePath, TRUE);
		if(!NT_SUCCESS(ntStatus)){
			DbgPrint("[-] Error :: ArmaditoGuard!SendScanOrder :: RtlUnicodeStringToAnsiString() routine failed !! \n");
			__leave;
		}

		// Fill the SCAN_MESSAGE_CONTEXT struct.
		RtlCopyMemory(&(scanMessage->FileName), AnsiString.Buffer, AnsiString.Length);
		scanMessage->FileNameLength = AnsiString.Length;
		scanMessage->scan_result = NONE;


		// Set timeout (1second).
		liTimeOut.QuadPart = -100000000LL;
		//liTimeOut.QuadPart = -50000000LL; // 0.1 sec
		//timeOut = &liTimeOut;
		timeOut.QuadPart = -(localScanTimeout * _1ms);


		//replyLength = sizeof(FILTER_REPLY_HEADER) + sizeof(SCAN_RESULT);
		replyLength = sizeof(SCAN_RESULT);
		// Send message to the scan process throught the communication port.
		ntStatus = FltSendMessage(FilterHandle, &gClientComPort, (PVOID)scanMessage, sizeof(MESSAGE_CONTEXT), (PVOID)&replyMessage, &replyLength, &timeOut);
		if (!NT_SUCCESS(ntStatus)) {
			DbgPrint("[-] Error :: ArmaditoGuard!SendScanOrder :: FltSendMessage() routine failed !! \n");
			__leave;
		}


		if(ntStatus == STATUS_TIMEOUT){
			//DbgPrint("[-] Warning :: ArmaditoGuard!SendScanOrder :: FltSendMessage() returned with timeout status !! \n");
			*AnswerMessage = TIMEOUT;
		}
		else {
			*AnswerMessage = replyMessage;
			//DbgPrint("[+] Debug :: ArmaditoGuard!SendScanOrder :: FltSendMessage() executed successfully !! :: Scan Result = [TODO] \n");
			if (replyMessage != NONE) {
				;//DbgPrint("[+] DEBUG :: ArmaditoGuard!SendScanOrder :: Reply message received successfully from the scan service :: Scan Result = [DBG_FLAG] \n");
			}
			else {
				DbgPrint("[+] Warning :: ArmaditoGuard!SendScanOrder :: Reply message not received completly...\n");
			}
		}
		
	}
	__finally {
		
		//
		//	Whatever happens, this buffer is not required anymore after this routine.
		//
		if(scanMessage != NULL){
			ExFreePoolWithTag(scanMessage, MESSAGE_CONTEXT_BUF);
			scanMessage = NULL;
		}

		//
		//	Free ansi string if this one has been allocated.
		//
		if(AnsiString.Buffer != NULL){
			RtlFreeAnsiString(&AnsiString);
		}


	}
	


	
	return ntStatus;

}
