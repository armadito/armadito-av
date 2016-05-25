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

#ifndef _LIBARMADITO_OS_MIMETYPE_H_
#define _LIBARMADITO_OS_MIMETYPE_H_

#ifdef __cplusplus
extern "C" {
#endif


/**
 *      \fn void os_mime_type_init(void);
 *      \brief Initializes mime type detection
 *
 *      Must be called prior to any call to os_mime_type_guess()
 *      This function must be safe w.r.t. multiple calls.
 *
 */
void os_mime_type_init(void);

/**
 *      \fn const char *os_mime_type_guess(const char *path);
 *      \brief Returns the mime type of a file
 *
 *      \param[in] path the path of the file
 *
 *      \return the mime type as a string, NULL if not guessable
 */
const char *os_mime_type_guess(const char *path);

/**
 *      \fn const char *os_mime_type_guess_fd(intfd);
 *      \brief Returns the mime type of a file given by a file descriptor
 *
 *      \param[in] fd file descriptor of the opened file
 *
 *      \return the mime type as a string, NULL if not guessable
 */
const char *os_mime_type_guess_fd(int fd);

#ifdef __cplusplus
}
#endif

#endif
