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
 * @file   table.h
 *
 */

#ifndef TABLE_H
#define TABLE_H

#include "windowsTypes.h"
#include "h1-errors.h"

/* these defines are used in order to characterize the model of an element */
#define 	ELEMENT_NO_MODEL 			 0
#define 	ELEMENT_MALWARE_MODEL 		 1
#define 	ELEMENT_NOT_MALWARE_MODEL 	-1
#define 	ELEMENT_MODEL_ERROR			 2

/*
*the table structure is used to store the k nearest neighbours
*the table structure is:
*-an int representing its size
*-an array of 'element', representing all the neighbours
*
*-the element structure is:
*-a double representing the distance between the neighbour and the testing file
*-a char representing the origin of the neighbour, defined by the calling function, with th default value being ELEMENT_NO_MODEL
*/

struct element{
	DOUBLE dist;
	CHAR model;
};

typedef struct _table{
	DWORD n;
	struct element* e;
} TABLE, *PTABLE;

/**
 * Create a new table of size n
 * @param  n a DWORD, the size of the table
 * @return   the table of size n or NULL if an error occured
 */
PTABLE tableNew(DWORD n);

/**
 * Add an element to the table, or replace the greater element if the table is full and if the new element is smaller.
 * @param  t     the table where we want to add an element
 * @param  dist  the dist field of the element we want to add
 * @param  model the model field of the element we want to add
 * @return       ARMADITO_SUCCESS or E_FAILURE
 */
ERROR_CODE tableAddElement(PTABLE t, DOUBLE dist, CHAR model);

/**
 * Memory freeing of the table passed in argument
 */
VOID tableDelete(PTABLE t);

/**
 * Return the dist field of the element i of the table t
 * @param  i the element we want to have the dist field
 * @return   the value of the dist field or E_DISTANCE_ERROR
 */
DOUBLE tableElementDistance(PTABLE t, DWORD i);

/**
 * Return the model field of the element i of the table t
 * @param  i the element we want to have the model field
 * @return   the value of the model field or ELEMENT_MODEL_ERROR
 */
CHAR tableElementModel(PTABLE t, DWORD i);

/**
 * Print the table
 */
VOID tableShow(PTABLE t);

#endif /* TABLE_H */
