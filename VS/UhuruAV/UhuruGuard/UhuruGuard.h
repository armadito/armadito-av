#ifndef _UHURU_GUARD_H_
#define _UHURU_GUARD_H_

#define QUARANTINE_DIR L"\\Program Files\\Novit\\UhuruAV\\Quarantine\\"

//Debug mode.
#define UH_DEBUG 0

#ifdef UH_DEBUG
	#define uhDbgPrint DbgPrint
#else
	#define uhDbgPrint 
#endif

#endif