#ifndef __ARMADITO_GUARD_H__
#define __ARMADITO_GUARD_H__

//Debug mode.
#define A60_DEBUG 0

#ifdef A60_DEBUG
	#define a6oDbgPrint DbgPrint
#else
	#define a6oDbgPrint 
#endif

#endif