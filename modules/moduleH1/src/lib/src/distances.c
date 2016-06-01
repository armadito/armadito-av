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
 * @file   distances.c
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "distances.h"
#include "h1-errors.h"

DOUBLE distanceDFG(PVECTOR a, PVECTOR b){
	DWORD i = 0, j = 0, m = 0, n = 0;
	DOUBLE sum = 0;
	VEC_TYPE a_i = 0, b_j = 0;

	m = vectorGetSize(a);
	n = vectorGetSize(b);

	/* if one of the vector is empty (m or n equal 0) or NULL (m or n equal -1), return error code E_DISTANCE_ERROR (which is >1) */
	if (m*n <= 0) {
		return E_DISTANCE_ERROR;
	}

	while (i < m && j < n){
		a_i = vectorGetValue(a, i);
		b_j = vectorGetValue(b, j);

		if ((a_i == (VEC_TYPE)-1) || (b_j == (VEC_TYPE)-1)){
			return E_DISTANCE_ERROR;
		}

		if (a_i == b_j){
			i++; j++;
		}
		else if (a_i > b_j){
			sum++; j++;
		}
		else if (a_i < b_j){
			sum++; i++;
		}
	}

	sum += m + n - i - j;

	/* normalization */
	return sum / ((DOUBLE)m + n);
}
