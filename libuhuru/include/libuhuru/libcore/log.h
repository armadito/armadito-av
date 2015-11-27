/**
 * \file log.h
 *
 * \brief definition of log function and log handling
 *
 * Standard method for logging in a platform independant way
 *
 */

#ifndef _LIBUHURU_LIBCORE_LOG_H_
#define _LIBUHURU_LIBCORE_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \enum uhuru_log_level
 * \brief the log level
 *
 * The log level is used to differentiate logged messages. 
 * A current log level allows to select dynamically which messages are displayed, 
 * from just error messages up to all debug messages.
 * A special level, LOG_LEVEL_NONE, is always displayed, independently of the 
 * current log level.
 * 
 */
enum uhuru_log_level {
  UHURU_LOG_LEVEL_ERROR      = 1 << 1,   /*!< error, operations must stop                   */
  UHURU_LOG_LEVEL_WARNING    = 1 << 2,   /*!< warning, things can go on                     */
  UHURU_LOG_LEVEL_INFO       = 1 << 3,   /*!< normal information                            */
  UHURU_LOG_LEVEL_DEBUG      = 1 << 4,   /*!< debug message                                 */
  UHURU_LOG_LEVEL_NONE       = 1 << 5,   /*!< no specific level, message will always show   */
};

/**
 * \enum uhuru_log_domain
 * \brief the log domain
 * 
 */
enum uhuru_log_domain {
  UHURU_LOG_LIBUHURU,          /*!< messages from the library                     */
  UHURU_LOG_MODULE,            /*!< messages from the modules                     */
  UHURU_LOG_SERVICE,           /*!< messages from the service or daemon           */
};

/**
 * \fn void uhuru_log(enum uhuru_log_domain domain, enum uhuru_log_level level, const char *format, ...);
 * \brief logs a message
 *
 * Logs a message using the current log handler.
 * If level is greater than the max_level of the log handler, message is not displayed.
 *
 * \param[in] domain               the log domain
 * \param[in] level                the log level
 * \param[in] format               printf like format
 *
 */
void uhuru_log(enum uhuru_log_domain domain, enum uhuru_log_level level, const char *format, ...);

typedef void (*uhuru_log_handler_t)(enum uhuru_log_level log_level, const char *message, void *user_data);

/**
 * \fn void uhuru_log_set_handler(enum uhuru_log_level max_level, uhuru_log_handler_t handler, void *user_data);
 * \brief set a log handler
 *
 * Register a log handler for the log levels smaller than max_level, i.e. if max_level is 
 * UHURU_LOG_LEVEL_WARNING, only calls to uhuru_log with a log level of ERROR or WARNING will
 * output a message
 *
 * \param[in] max_level            the maximum log level
 * \param[in] handler              the handler
 * \param[in] user_data            generic pointer that will be passed to the handler
 *
 */
void uhuru_log_set_handler(enum uhuru_log_level max_level, uhuru_log_handler_t handler, void *user_data);

#ifdef __cplusplus
}
#endif

#endif
