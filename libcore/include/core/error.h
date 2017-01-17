/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

/**
 * \file error.h
 *
 * \brief definition of error handling
 *
 * Standard method of reporting errors from a called function to the calling code.
 *
 * A a6o_error structure contains:
 * - an error code specific to the domain (module, scan, info...)
 * - a formatted error message
 *
 * The error code possible values are:
 * - a value of 0, meaning NO error
 * - a value != 0, meaning a real error
 */

#ifndef ARMADITO_CORE_ERROR_H
#define ARMADITO_CORE_ERROR_H

#include <stdio.h>  /* for FILE * */

/**
 * \var typedef struct _a6o_error a6o_error;
 * \struct struct _a6o_error
 *
 * \brief a structure containing an error
 *
 * The `a6o_error` structure contains information about an error that has occurred.
 *
 */

typedef struct _a6o_error {
	int error_code;
	const char *error_message;
} a6o_error;

/**
 * \fn a6o_error *a6o_error_new(int error_code, const char *error_message);
 * \brief allocates an a6o_error structure
 *
 * This function uses malloc() to allocate the structure.
 * The message is NOT strdup'ed.
 *
 * \param[in] error_code           the error code
 * \param[in] error_message        the error message
 *
 * \return a pointer to the allocated structure
 */
a6o_error *a6o_error_new(int error_code, const char *error_message);

/**
 * \fn void a6o_error_set(a6o_error **error, int error_code, const char *error_message);
 * \brief set an error
 *
 * Does nothing if error is NULL; if error is non NULL, then *error must be NULL (i.e. errors can't be
 * assigned twice).
 * A new a6o_error is created and assigned to *error .
 * The message is NOT strdup'ed.
 *
 * \param[in] error                return location for a a6o_error code, or NULL
 * \param[in] error_code           the error code
 * \param[in] error_message        the error message
 *
 */
void a6o_error_set(a6o_error **error, int error_code, const char *error_message);

/**
 * \fn void a6o_error_free(a6o_error *err);
 * \brief frees an a6o_error structure
 *
 * \param[in] err            a pointer to the a6o_error struct, if NULL, function does nothing
 */
void a6o_error_free(a6o_error *err);

/**
 * \fn void a6o_error_print(a6o_error *err, FILE *out)
 * \brief prints an a6o_error structure on `out`
 *
 * \param[in] err            a pointer to the a6o_error struct, if NULL, function does nothing
 * \param[in] out            a pointer to the FILE output
 */
void a6o_error_print(a6o_error *err, FILE *out);

/*
 * Error codes for module
 */
#define ARMADITO_ERROR_MODULE                      100
#define ARMADITO_ERROR_MODULE_INIT_FAILED          (ARMADITO_ERROR_MODULE + 1)
#define ARMADITO_ERROR_MODULE_POST_INIT_FAILED     (ARMADITO_ERROR_MODULE + 2)
#define ARMADITO_ERROR_MODULE_CLOSE_FAILED         (ARMADITO_ERROR_MODULE + 3)
#define ARMADITO_ERROR_MODULE_SYMBOL_NOT_FOUND     (ARMADITO_ERROR_MODULE + 4)

/*
 * Error codes for scan
 */
#define ARMADITO_ERROR_SCAN                        200

/*
 * Error codes for configuration
 */
#define ARMADITO_ERROR_CONF                        300
#define ARMADITO_ERROR_CONF_FILE_NOT_FOUND         (ARMADITO_ERROR_CONF + 1)
#define ARMADITO_ERROR_CONF_SYNTAX_ERROR           (ARMADITO_ERROR_CONF + 1)

#endif
