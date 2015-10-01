#ifndef _LIBUHURU_OS_IPC_H_
#define _LIBUHURU_OS_IPC_H_

/**
 *      \fn int os_ipc_connect(const char *url);
 *      \brief connect to a local server using platform's IPC
 *      IPC is implemented with Unix sockets on Linux, named pipes on Windows
 *
 *      \param[in] url the url (path for Unix socket, url for Windows named pipe)
 *      \param[in] max_retry maximum number of retries
 *
 *      \return a file descriptor
 */
int os_ipc_connect(const char *url, int max_retry);

#endif

