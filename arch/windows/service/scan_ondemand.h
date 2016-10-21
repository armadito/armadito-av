/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef __SCAN_ON_DEMAND_H__
#define __SCAN_ON_DEMAND_H__

#include "structs.h"

#define SVC_IPC_PATH "\\\\.\\pipe\\Armadito_ondemand"
#define UI_IPC_PATH "\\\\.\\pipe\\Armadito-UI"
#define SCAN_THREAD_COUNT 3
#define BUFSIZE 512


/* Functions prototypes*/
int Start_IHM_Connection(_Inout_ PGLOBAL_SCAN_CONTEXT Context);
int WINAPI MainThreadWork(PGLOBAL_SCAN_CONTEXT Context);
int WINAPI ScanThreadWork(PGLOBAL_SCAN_CONTEXT Context);
int Close_IHM_Connection(_In_ PGLOBAL_SCAN_CONTEXT Context);
int TerminateAllScanThreads(PGLOBAL_SCAN_CONTEXT Context);


#endif