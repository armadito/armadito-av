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
#ifndef _LIBARMADITO_HANDLE_H_
#define _LIBARMADITO_HANDLE_H_

#include <libarmadito/error.h>
#include <libarmadito/conf.h>

/**
 * \struct struct armadito
 * \brief an opaque structure that is the basic for all armadito operations
 *
 */
struct armadito;

/**
 * \fn struct armadito *a6o_open(struct a6o_conf *conf, a6o_error **error);
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
 * If any step fails, the function returns NULL and fills the error if error return location is non NULL.
 *
 * \param[in] conf       the configuration
 * \param[out] error     return location for a a6o_error, or NULL
 *
 * \return               a pointer to the allocated handle, or NULL if error occured
 */
struct armadito *a6o_open(struct a6o_conf *conf, a6o_error **error);

/**
 * \fn struct armadito *a6o_open(a6o_error **error);
 * \brief de-initialize and de-allocate a armadito handle
 *
 * This function de-initializes and de-allocates a `armadito` structure.
 *
 * De-initialization steps are:
 * - calling the `close` function of each module
 * - de-allocating the module data
 *
 * If any step fails, the function returns an error code and fills the error if error return location is non NULL.
 *
 * \param[out] error     return location for a a6o_error, or NULL
 *
 * \return               0 if OK, error code if an error occured
 */
int a6o_close(struct armadito *u, a6o_error **error);

struct a6o_conf *a6o_get_conf(struct armadito *u);

#endif
