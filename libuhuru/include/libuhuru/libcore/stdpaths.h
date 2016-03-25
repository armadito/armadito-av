/**
 * \file stdpaths.h
 *
 * \brief definition of Armadito standard paths, for modules, configuration...
 *
 * The std_path() function returns for accessing standard paths.
 *
 * This file defines functions to query standard locations on the local filesystem, such as 
 * - modules binaries directory
 * - configuration file
 * - configuration directory
 * - bases directory
 *
 * Standard locations are platform dependant.
 *
 */

#ifndef _LIBUHURU_LIBCORE_STDPATHS_H_
#define _LIBUHURU_LIBCORE_STDPATHS_H_
#ifdef __cplusplus
extern "C" {
#endif

  enum uhuru_std_location {
    MODULES_LOCATION,
    CONFIG_FILE_LOCATION,
    CONFIG_DIR_LOCATION,
    BASES_LOCATION,
    BINARY_LOCATION,
    TMP_LOCATION,
    LAST_LOCATION,
  };

  const char *uhuru_std_path(enum uhuru_std_location location);

#ifdef __cplusplus
}
#endif

#endif
