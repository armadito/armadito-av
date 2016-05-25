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
 * @file   distances.h
 *
 */

#ifndef DISTANCES_H
#define DISTANCES_H

#include "vector.h"
#include "windowsTypes.h"

/**
 * Compute the dfg distance between a and b
 * @param  a a PVECTOR
 * @param  b a PVECTOR
 * @return   the distance between a and b, from 0 (equal) to 1 (different), or E_DISTANCE_ERROR if there was an error.
 */
DOUBLE distanceDFG(PVECTOR a, PVECTOR b);

#endif /* DISTANCES_H */
