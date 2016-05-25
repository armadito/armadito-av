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
 * @file   table.c
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "table.h"

PTABLE tableNew(DWORD n){
	DWORD i = 0;
	PTABLE t;

	t = (PTABLE)calloc(1, sizeof(TABLE));
	if (t == NULL){
		return NULL;
	}
	t->e = (struct element*) calloc(n, sizeof(struct element));
	if (t->e == NULL){
		free(t);
		return NULL;
	}
	t->n = n;

	/* table initialization */
	for (i = 0; i < n; i++){
		t->e[i].dist = 1;
		t->e[i].model = ELEMENT_NO_MODEL;
	}

	return t;
}

DWORD tableMaxElement(PTABLE t){
	DWORD i = 0, j = 0;

	if (t == NULL){
		return (DWORD)-1;
	}

	for (j = 0; j < t->n; j++){
		if (t->e[j].model == ELEMENT_NO_MODEL){ /*if an unused element is found*/
			return j;
		}

		if (t->e[j].dist > t->e[i].dist){
			i = j;
		}
	}

	return i;
}

ERROR_CODE tableAddElement(PTABLE t, DOUBLE dist, CHAR model){
	DWORD i = 0;

	i = tableMaxElement(t);
	if (i == ((DWORD)-1)){
		return E_FAILURE;
	}

	/* the element is added only if it has a dist inferior to the dist of the max element, or if the table is not full */
	if (dist < t->e[i].dist || t->e[i].model == ELEMENT_NO_MODEL){
		t->e[i].dist = dist;
		t->e[i].model = model;
	}

	return ARMADITO_SUCCESS;
}

VOID tableDelete(PTABLE t){
	free(t->e);
	free(t);
}

CHAR tableElementModel(PTABLE t, DWORD i){
	if (t == NULL || i >= t->n){
		return ELEMENT_MODEL_ERROR;
	}
	return t->e[i].model;
}

DOUBLE tableElementDistance(PTABLE t, DWORD i){
	if (t == NULL || i >= t->n){
		return E_DISTANCE_ERROR;
	}
	return t->e[i].dist;
}

VOID tableShow(PTABLE t){
	DWORD j;
	for (j = 0; j < t->n; j++){
		printf("[*] Element %2lu : Distance %1.5f & model %d\n", j, t->e[j].dist, t->e[j].model);
	}
}
