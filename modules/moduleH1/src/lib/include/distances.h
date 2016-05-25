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
