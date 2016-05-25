/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito windows driver.

Reproduction, distribution and derivative works are permitted under the terms of the Microsoft Public License
See file COPYING.MSPL for terms of license.

***/

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include "structs.h"


FLT_PREOP_CALLBACK_STATUS
PreOperationIrpCreate(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
);


FLT_POSTOP_CALLBACK_STATUS
PostOperationIrpCreate(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags);

FLT_POSTOP_CALLBACK_STATUS
PostOperationIrpWrite(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags);


FLT_PREOP_CALLBACK_STATUS
PreOperationIrpCleanup(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
);


FLT_POSTOP_CALLBACK_STATUS
PostOperationIrpCleanup(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags);






#endif