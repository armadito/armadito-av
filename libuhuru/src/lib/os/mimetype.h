#ifndef _LIBUHURU_OS_MIMETYPE_H_
#define _LIBUHURU_OS_MIMETYPE_H_

/**
 *      \fn const char *mime_type_guess(const char *path);
 *      \brief Returns the mime type of a file
 *
 *      \param[in] path the path of the file
 *
 *      \return the mime type as a string, NULL if not guessable
 */
const char *mime_type_guess(const char *path);

#endif
