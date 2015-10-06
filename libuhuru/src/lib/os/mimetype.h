#ifndef _LIBUHURU_OS_MIMETYPE_H_
#define _LIBUHURU_OS_MIMETYPE_H_

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
 *      \fn void os_mime_type_init(void);
 *      \brief Initializes mime type detection
 *
 *      Must be called prior to any call to os_mime_type_guess()
 *      This function must be safe w.r.t. multiple calls.
 *
 */
void os_mime_type_init(void);

#endif
