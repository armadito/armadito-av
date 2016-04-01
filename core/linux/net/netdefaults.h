#ifndef _NETDEFAULTS_H_
#define _NETDEFAULTS_H_

/* unix sockets by default, no tcp */
#define DEFAULT_SOCKET_TYPE   "unix"

/* for compatibility with node.js, we don't use abstract socket address (see man 7 unix) */
/* despite the fact that they have the advantage that they do not require to remove the  */
/* file before bind()ing */
#define DEFAULT_SOCKET_PATH   "/tmp/.armadito-daemon"
/* use this definition for abstract socket address */
/* #define DEFAULT_SOCKET_PATH   "@/tmp/.armadito-daemon" */

/* for tcp socket only */
#define DEFAULT_SOCKET_PORT   "14444"

#endif
