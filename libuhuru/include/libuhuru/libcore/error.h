/**
 * \file scan.h
 *
 * \brief definition of error handling
 *
 * Standard method of reporting errors from a called function to the calling code.
 *
 * A uhuru_error structure contains:
 * - an error code specific to the domain (module, scan, info...)
 * - a formatted error message
 *
 * The error code possible values are:
 * - a value of 0, meaning NO error
 * - a value != 0, meaning a real error
 */

#ifndef _LIBUHURU_LIBCORE_ERROR_H_
#define _LIBUHURU_LIBCORE_ERROR_H_

/**
 * \var typedef struct _uhuru_error uhuru_error;
 * \struct struct _uhuru_error
 *
 * \brief a structure containing an error
 *
 * The `uhuru_error` structure contains information about an error that has occurred.
 *
 */

typedef struct _uhuru_error {
  int error_code;
  const char *error_message;
} uhuru_error;

/**
 * \fn uhuru_error *uhuru_error_new(int error_code, const char *error_message);
 * \brief allocates an uhuru_error structure
 *
 * This function uses malloc() to allocate the structure.
 * The message is NOT strdup'ed.
 *
 * \param[in] error_code           the error code
 * \param[in] error_message        the error message
 *
 * \return a pointer to the allocated structure
 */
uhuru_error *uhuru_error_new(int error_code, const char *error_message);

/**
 * \fn void uhuru_error_set(uhuru_error **error, int error_code, const char *error_message);
 * \brief set an error
 *
 * Does nothing if error is NULL; if error is non NULL, then *error must be NULL (i.e. errors can't be
 * assigned twice).
 * A new uhuru_error is created and assigned to *error .
 * The message is NOT strdup'ed.
 *
 * \param[in] error                return location for a uhuru_error code, or NULL
 * \param[in] error_code           the error code
 * \param[in] error_message        the error message
 *
 */
void uhuru_error_set(uhuru_error **error, int error_code, const char *error_message);

/**
 * \fn void uhuru_error_free(uhuru_error *err);
 * \brief frees an uhuru_error structure
 *
 * \param[in] err            a pointer to the uhuru_error struct
 */
void uhuru_error_free(uhuru_error *err);


/*
 * Error codes for module
 */
#define UHURU_ERROR_MODULE                      100
#define UHURU_ERROR_MODULE_INIT_FAILED          (UHURU_ERROR_MODULE + 1)
#define UHURU_ERROR_MODULE_POST_INIT_FAILED     (UHURU_ERROR_MODULE + 2)
#define UHURU_ERROR_MODULE_CLOSE_FAILED         (UHURU_ERROR_MODULE + 3)

/*
 * Error codes for scan
 */
#define UHURU_ERROR_SCAN                        200

/*
 * Error codes for configuration
 */
#define UHURU_ERROR_CONF                        300
#define UHURU_ERROR_CONF_FILE_NOT_FOUND         (UHURU_ERROR_CONF + 1)
#define UHURU_ERROR_CONF_SYNTAX_ERROR           (UHURU_ERROR_CONF + 1)

#endif
