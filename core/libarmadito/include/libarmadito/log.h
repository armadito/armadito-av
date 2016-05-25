/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

/**
 * \file log.h
 *
 * \brief definition of log function and log handling
 *
 * Standard method for logging in a platform independant way
 *
 */

#ifndef _LIBARMADITO_LOG_H_
#define _LIBARMADITO_LOG_H_

/**
 * \enum a6o_log_level
 * \brief the log level
 *
 * The log level is used to differentiate logged messages.
 * The log facility maintains a 'current log level', which allows to select dynamically
 * which messages are displayed, from just error messages up to all debug messages.
 * A special level, LOG_LEVEL_NONE, is always displayed, independently of the
 * current log level.
 *
 */
enum a6o_log_level {
	ARMADITO_LOG_LEVEL_ERROR      = 1 << 1,   /*!< error, operations must stop                   */
	ARMADITO_LOG_LEVEL_WARNING    = 1 << 2,   /*!< warning, things can go on                     */
	ARMADITO_LOG_LEVEL_INFO       = 1 << 3,   /*!< normal information                            */
	ARMADITO_LOG_LEVEL_DEBUG      = 1 << 4,   /*!< debug message                                 */
	ARMADITO_LOG_LEVEL_NONE       = 1 << 5,   /*!< no specific level, message will always show   */
};

/**
 * \enum a6o_log_domain
 * \brief the log domain
 *
 */
enum a6o_log_domain {
	ARMADITO_LOG_LIB,               /*!< messages from the library                     */
	ARMADITO_LOG_MODULE,            /*!< messages from the modules                     */
	ARMADITO_LOG_SERVICE,           /*!< messages from the service or daemon           */
};

/**
 * \fn void a6o_log(enum a6o_log_domain domain, enum a6o_log_level level, const char *format, ...);
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
void a6o_log(enum a6o_log_domain domain, enum a6o_log_level level, const char *format, ...);

typedef void (*a6o_log_handler_t)(enum a6o_log_domain domain, enum a6o_log_level log_level, const char *message, void *user_data);

/**
 * \fn void a6o_log_set_handler(enum a6o_log_level max_level, a6o_log_handler_t handler, void *user_data);
 * \brief set a log handler
 *
 * Register a log handler and set the current log level.
 * Only log levels smaller than max_level will be displayed, for example if max_level is
 * ARMADITO_LOG_LEVEL_WARNING, only calls to a6o_log with a log level of ERROR or WARNING will
 * output a message
 *
 * \param[in] max_level            the maximum log level
 * \param[in] handler              the handler
 * \param[in] user_data            generic pointer that will be passed to the handler
 *
 */
void a6o_log_set_handler(enum a6o_log_level max_level, a6o_log_handler_t handler, void *user_data);

const char *a6o_log_level_str(enum a6o_log_level log_level);

void a6o_log_default_handler(enum a6o_log_domain domain, enum a6o_log_level log_level, const char *message, void *user_data);

#endif
