/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito module H1.

Armadito module H1 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito module H1 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito module H1.  If not, see <http://www.gnu.org/licenses/>.

***/

/**
 * @file   vector.h
 *
 * This file define the type VECTOR, and all associated functions.
 */

#ifndef _VECTOR_H
#define _VECTOR_H

#include "windowsTypes.h"

typedef QWORD VEC_TYPE;

/*
*vector structure:
*-an array with the datas of type VEC_TYPE
*-an DWORD representing the size of the vector : if possible, don't set this to (DWORD)-1, since its the code for size error
*/
typedef struct _VECTOR{
	DWORD n;
	VEC_TYPE* v;
} VECTOR, *PVECTOR;

/**
 * create a new empty PVECTOR of size n
 * @param  n the size of the vector
 * @return   the created PVECTOR, or NULL if an error occured
 */
PVECTOR vectorNew(DWORD n);

/**
 * create a new PVECTOR from an existing array
 * @param  QwordArray the base array
 * @param  n          the size of the vector/size of the array
 * @return            the created PVECTOR, or NULL if an error occured
 */
PVECTOR VectorCreateFromArray(VEC_TYPE* QwordArray, DWORD n);

/**
 * return the size of a PVECTOR
 * @param  v the PVECTOR we want to know the size
 * @return   the size, or (DWORD)-1 if the vector is NULL
 */
DWORD vectorGetSize(PVECTOR v);

/**
 * get the ith element of the v PVECTOR
 * @param  v a PVECTOR
 * @param  i a DWORD
 * @return   the i-th element of v (v[i]), or (VEC_TYPE)-1 if the vector is NULL or if i > n
 */
VEC_TYPE vectorGetValue(PVECTOR v, DWORD i);

/**
 * get the ith element of the v PVECTOR, but with v considered as padded with 0, which cause this to returns 0 if i > size of v
 * @param  v a PVECTOR
 * @param  i a DWORD
 * @return   0 if i > n, the i-th element of v (v[i]), or (VEC_TYPE)-1 if the vector is NULL
 */
VEC_TYPE vectorGetValueWithPaddedVector(PVECTOR v, DWORD i);

/**
 * free the vector
 * @param  x a PVECTOR
 */
VOID vectorDelete(PVECTOR x);

/**
 * load a file into a vector by reading element of size : sizeof(VEC_TYPE)
 * @param  fileName the file to load
 * @return          the created vector, or NULL if an error occured
 */
PVECTOR vectorLoad(CHAR* fileName);

#endif
