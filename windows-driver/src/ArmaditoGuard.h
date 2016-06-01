/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito windows driver.

Reproduction, distribution and derivative works are permitted under the terms of the Microsoft Public License
See file COPYING.MSPL for terms of license.

***/

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