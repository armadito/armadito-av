/**
 * @file   utils.h
 *
 * This file define some generic macros and constants used by the module.
 */

#ifndef UTILS_H
#define UTILS_H

#define TODO

#define		TRUE				1
#define		FALSE				0
/* maximum size of a constant CHAR[] */
#define 	NAMESIZE 			2048
/* number of nearest neighbors for the k-NN test */
#define 	NUMBER_OF_N_N		9
/* threshold for the iat test */
#define 	IAT_KNN_THRESHOLD	0.4
/* threshold for the eat test */
#define 	EAT_KNN_THRESHOLD	0

/**
 * MAX/MIN:
 * Those routines return the maximum or the minimum value between two variable a and b.
 */
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#endif /* UTILS_H */
