/**
 * \file handle.h
 *
 * \brief definition of uhuru handle
 *
 * A `uhuru handle` is a pointer to an opaque structure that contains
 * the `uhuru modules`.
 * TO BE COMPLETED
 *
 */
#ifndef _LIBUHURU_LIBCORE_HANDLE_H_
#define _LIBUHURU_LIBCORE_HANDLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libuhuru/libcore/error.h>
#include <libuhuru/libcore/conf.h>

  /**
   * \struct struct uhuru
   * \brief an opaque structure that is the basic for all uhuru operations
   *
   */
  struct uhuru;

  /**
   * \fn struct uhuru *uhuru_open(struct uhuru_conf *conf, uhuru_error **error);
   * \brief allocate and initialize a uhuru handle
   *
   * This function allocates a `uhuru` structure and initializes it.
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
   * \param[out] error     return location for a uhuru_error, or NULL
   *
   * \return               a pointer to the allocated handle, or NULL if error occured
   */
  struct uhuru *uhuru_open(struct uhuru_conf *conf, uhuru_error **error);

  /**
   * \fn struct uhuru *uhuru_open(uhuru_error **error);
   * \brief de-initialize and de-allocate a uhuru handle
   *
   * This function de-initializes and de-allocates a `uhuru` structure.
   * 
   * De-initialization steps are:
   * - calling the `close` function of each module
   * - de-allocating the module data
   *
   * If any step fails, the function returns an error code and fills the error if error return location is non NULL.
   * 
   * \param[out] error     return location for a uhuru_error, or NULL
   *
   * \return               0 if OK, error code if an error occured
   */
  int uhuru_close(struct uhuru *u, uhuru_error **error);

#ifdef __cplusplus
}
#endif

#endif
