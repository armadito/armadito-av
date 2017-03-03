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
 * \file handle.h
 *
 * \brief definition of armadito handle
 *
 * A `armadito handle` is a pointer to an opaque structure that contains
 * the `armadito modules`.
 * TO BE COMPLETED
 *
 */
#ifndef ARMADITO_CORE_HANDLE_H
#define ARMADITO_CORE_HANDLE_H

#include <core/conf.h>

/**
 * \struct struct armadito
 * \brief an opaque structure that is the basic for all armadito operations
 *
 */
struct armadito;

/**
 * \fn struct armadito *a6o_open(struct a6o_conf *conf);
 * \brief allocate and initialize a armadito handle
 *
 * This function allocates a `armadito` structure and initializes it.
 *
 * Initialization steps are:
 * - dynamic loading of the module located in modules path (platform dependant)
 * - calling the `init` function of each module
 * - calling the configuration functions of each module for the given configuration
 * - calling the `post_init` function of each module
 *
 * - loading the configuration file located in configuration path (platform dependant)
 *
 *
 * If any step fails, the function returns NULL.
 *
 * \param[in] conf       the configuration
 *
 * \return               a pointer to the allocated handle, or NULL if error occured
 */
struct armadito *a6o_open(struct a6o_conf *conf);

/**
 * \fn int a6o_close(struct armadito *u)
 * \brief de-initialize and de-allocate a armadito handle
 *
 * This function de-initializes and de-allocates a `armadito` structure.
 *
 * De-initialization steps are:
 * - calling the `close` function of each module
 * - de-allocating the module data
 *
 * If any step fails, the function returns an error code.
 *
 * \return               0 if OK, error code if an error occured
 */
int a6o_close(struct armadito *u);

struct a6o_conf *a6o_get_conf(struct armadito *u);

struct a6o_event_source *a6o_get_event_source(struct armadito *u);

#endif
