/**
 * \file ipc.h
 *
 * \brief definition of the IPC protocol and API
 * 
 * The IPC protocol is a binary protocol based on "messages".
 * 
 * A "message" is a sequence of bytes containing:
 * - a message identifier, or MESSAGE_ID (one byte)
 * - a variable number of typed arguments
 * - a END_OF_MESSAGE marker (one byte)
 * 
 * +---------------+------//----+----//----+     +-----------------+
 * |               |  	        |    	   |     |                 |
 * |  MESSAGE ID   |  argument  | argument | ... |  END OF MESSAGE |
 * |  (one byte)   |  	        |    	   |     |    (one byte)   |
 * +---------------+------//----+----//----+     +-----------------+ 	       	      
 * 
 * A message "argument" is a sequence of bytes containing:
 * - a type identifier (one byte)
 * - a payload containing the argument value (variable number of bytes,
 * that depends on argument type)
 * 
 * +---------------+-----------+-----------+
 * |               |  	       |  	   |
 * |   TYPE ID     |  payload  |  payload  | ... 
 * |  (one byte)   |  byte 1   |  byte 2   |
 * +---------------+-----------+-----------+
 * 
 * Currently supported argument types are:
 * - IPC_TYPE_STRING (string type): payload is a null-terminated sequence 
 * of bytes
 * - IPC_TYPE_INT32 (32 bits integer): payload is a sequence of 4 bytes
 * in platform natural byte order
 * 
 * The IPC API functions and macros defines
 * - macros for message IDs and argument type IDs
 * - functions for protocol encoding/decoding
 *
 * The IPC API makes no assumption about the communication support used
 * to send and receive messages. This support can be:
 * - a Unix socket for Unix-like systems
 * - a Windows named pipe
 * - a TCP socket
 *
 * The API allows:
 * - to send messages using ipc_manager_msg_send() or its variants ipc_manager_msg_begin(),
 * ipc_manager_msg_add(), ipc_manager_msg_end()
 * - to install "handler" functions called when a message is received, after protocol decoding
 */

#ifndef _LIBUHURU_LIBIPC_IPC_H_
#define _LIBUHURU_LIBIPC_IPC_H_

/**
 * \var typedef int ipc_int32_t;
 * \brief type definitions for arguments 
 */
/* note: will probably need adjustment w.r.t. 32/64 bits */
typedef int ipc_int32_t;

/**
 * \var typedef unsigned char ipc_type_id_t;
 * \brief definition of the C type of an IPC type ID
 */
typedef unsigned char ipc_type_id_t;

/**
 * \brief definition of the currently supported type IDs
 */
#define IPC_TYPE_INT32            ((ipc_type_id_t)0x81)      /*!< 32 bits integer          */
#define IPC_TYPE_STRING           ((ipc_type_id_t)0x82)      /*!< null terminated string   */

/**
 * \brief definition of the "end of message" marker
 */
#define IPC_END_OF_MESSAGE        ((ipc_type_id_t)0x80)      /*!< end of message marker     */

#if 0
/* the following structure is probably not needed; it is usefull only for ipc_manager_get_argv() */
/* which was never used... */
so it is now moved inside ipc.c
/**
 * \struct struct ipc_arg
 * \brief C struct for message arguments
 *
 * Contains a type ID and an union for the various argument types
 */
#endif

/**
 * \brief definition of the C type of an IPC message ID
 */
typedef unsigned char ipc_msg_id_t;

/**
 * \brief definition of the currently supported message IDs
 */
#define IPC_MSG_ID_FIRST             ((ipc_msg_id_t)0)      /*!< first message ID, unused      */
#define IPC_MSG_ID_PING              ((ipc_msg_id_t)0)      /*!< ping message ID               */
#define IPC_MSG_ID_PONG              ((ipc_msg_id_t)1)      /*!< pong message ID               */
#define IPC_MSG_ID_SCAN              ((ipc_msg_id_t)2)      /*!< scan message ID               */
#define IPC_MSG_ID_SCAN_RESULT       ((ipc_msg_id_t)3)      /*!< scan result message ID        */
#define IPC_MSG_ID_SCAN_END          ((ipc_msg_id_t)4)      /*!< end of scan message ID        */
#define IPC_MSG_ID_INFO              ((ipc_msg_id_t)5)      /*!< send information message ID   */
#define IPC_MSG_ID_INFO_MODULE       ((ipc_msg_id_t)6)      /*!< module information message ID */
#define IPC_MSG_ID_INFO_END          ((ipc_msg_id_t)7)      /*!< end of information message ID */
#define IPC_MSG_ID_LAST              ((ipc_msg_id_t)7)      /*!< last message ID, unused       */

/**
 * \struct struct ipc_manager
 * \brief an opaque structure for protocol encoding/decoding
 */
struct ipc_manager;

/**
 * \var typedef int (ipc_write_fun_t)(void *g_handle, const char *buffer, unsigned int len);
 * \brief the type definition for a generic write function
 */
typedef int (ipc_write_fun_t)(void *g_handle, const char *buffer, unsigned int len);

/**
 * \var typedef int (ipc_read_fun_t)(void *g_handle, char *buffer, unsigned int len);
 * \brief the type definition for a generic read function
 */
typedef int (ipc_read_fun_t)(void *g_handle, char *buffer, unsigned int len);

/**
 * \fn struct ipc_manager *ipc_manager_new(void *g_handle, ipc_write_fun_t write_fun, ipc_read_fun_t read_fun);
 * \brief allocate and initialize an ipc_manager
 *
 * This function uses malloc() to allocate the structure
 *
 * \param[in] g_handle     the generic handle to underlying communication support (int fd for Unix, HANDLE hFile for Windows)
 * \param[in] write_fun    the generic write function for I/O
 * \param[in] read_fun     the generic read function for I/O
 */
struct ipc_manager *ipc_manager_new(void *g_handle, ipc_write_fun_t write_fun, ipc_read_fun_t read_fun);

/**
 * \fn void ipc_manager_free(struct ipc_manager *manager);
 * \brief de-initialize and free an ipc_manager
 *
 * \param[in] manager a pointer to the ipc_manager struct
 */
void ipc_manager_free(struct ipc_manager *manager);

/**
 * \fn ipc_msg_id_t ipc_manager_get_msg_id(struct ipc_manager *manager);
 * \brief returns current message ID
 *
 * This function is called inside an IPC handler function, to get current 
 * message ID.
 *
 * \param[in] manager a pointer to the ipc_manager struct
 * \return current message ID
 */
ipc_msg_id_t ipc_manager_get_msg_id(struct ipc_manager *manager);

/**
 * \fn int ipc_manager_get_argc(struct ipc_manager *manager);
 * \brief returns number of arguments for current message 
 *
 * This function is called inside an IPC handler function, to get arguments count
 * of current message
 *
 * \param[in] manager a pointer to the ipc_manager struct
 * \return number of arguments current message
 */
int ipc_manager_get_argc(struct ipc_manager *manager);

/* is this function usefull? */
/* struct ipc_value *ipc_manager_get_argv(struct ipc_manager *manager); */

/**
 * \fn int ipc_manager_get_arg_at(struct ipc_manager *manager, int index, ipc_type_id_t type, void *pvalue);
 * \brief returns an argument of the current message
 *
 * This function is called inside an IPC handler function, to get arguments of current message
 *
 * \param[in] manager a pointer to the ipc_manager struct
 * \param[in] index the index of argument to retrieve (must be >= 0 and < ipc_manager_get_argc())
 * \param[in] type the type ID of argument to retrieve
 * \param[in] pvalue a pointer to corresponding value

 * \return 0 if ok, -1 if error (index out of range or expected type not matching current type)
 */
int ipc_manager_get_arg_at(struct ipc_manager *manager, int index, ipc_type_id_t type, void *pvalue);

typedef void (*ipc_handler_t)(struct ipc_manager *manager, void *data);

/**
 * \fn int ipc_manager_add_handler(struct ipc_manager *manager, ipc_msg_id_t msg_id, ipc_handler_t handler, void *data);
 * \brief install a handler function for a message ID
 *
 * Note: only one message handler is allowed per message ID
 *
 * \param[in] manager a pointer to the ipc_manager struct
 * \param[in] msg_id the message ID for which to install the handler
 * \param[in] handler the handler function
 * \param[in] data a generic pointer that will be passed to the handler

 * \return 0 if ok, -1 if error (message ID out of range or handler already installed for this ID)
 */
int ipc_manager_add_handler(struct ipc_manager *manager, ipc_msg_id_t msg_id, ipc_handler_t handler, void *data);

/**
 * \fn int ipc_manager_receive(struct ipc_manager *manager);
 * \brief read and decode bytes from the underlying communication support
 *
 * Note: only one message handler is allowed per message ID
 *
 * \param[in] manager a pointer to the ipc_manager struct

 * \return 1 if ok, 0 if end of file reached, -1 if error
 */
int ipc_manager_receive(struct ipc_manager *manager);

/**
 * \fn int ipc_manager_msg_send(struct ipc_manager *manager, ipc_msg_id_t msg_id, ...);
 * \brief encode and send a message
 *
 * Arguments are passed in vararg style: type ID, argument payload, ... followed by
 * the end of message marker.
 * Example of user:
 * ipc_manager_msg_send(manager, IPC_MSG_ID_SCAN, IPC_TYPE_STRING, "c:\\Users\\joe", IPC_END_OF_MESSAGE);
 *
 * \param[in] manager a pointer to the ipc_manager struct
 * \param[in] msg_id the message ID to send
 * \return 1 if ok, 0 if error
 */
int ipc_manager_msg_send(struct ipc_manager *manager, ipc_msg_id_t msg_id, ...);

/**
 * \fn int ipc_manager_msg_begin(struct ipc_manager *manager, ipc_msg_id_t msg_id);
 * \brief encode a message ID
 *
 * Variant of ipc_manager_msg_send() that allows more flexibility
 *
 * \param[in] manager a pointer to the ipc_manager struct
 * \param[in] msg_id the message ID to send
 * \return 1 if ok, 0 if error
 */
int ipc_manager_msg_begin(struct ipc_manager *manager, ipc_msg_id_t msg_id);

/**
 * \fn int ipc_manager_msg_add(struct ipc_manager *manager, ipc_msg_id_t msg_id, ...);
 * \brief encode arguments
 *
 * Arguments are passed in vararg style: type ID, argument payload, ... followed by
 * the end of message marker.
 * Example of user:
 * ipc_manager_msg_add(manager, IPC_TYPE_STRING, "c:\\Users\\joe", IPC_END_OF_MESSAGE);
 * ipc_manager_msg_add(manager, IPC_TYPE_INT32, 42, IPC_TYPE_INT32, 666, IPC_END_OF_MESSAGE);
 *
 * \param[in] manager a pointer to the ipc_manager struct
 * \return 1 if ok, 0 if error
 */
int ipc_manager_msg_add(struct ipc_manager *manager, ...);

/**
 * \fn int ipc_manager_msg_end(struct ipc_manager *manager);
 * \brief add end of message marker and send the message
 *
 * \param[in] manager a pointer to the ipc_manager struct
 * \return 1 if ok, 0 if error
 */
int ipc_manager_msg_end(struct ipc_manager *manager);

#endif
