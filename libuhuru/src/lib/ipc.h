#ifndef _LIBUHURU_IPC_H_
#define _LIBUHURU_IPC_H_

#include <glib.h>

typedef guchar ipc_type_t;

#define IPC_NONE     ((ipc_type_t)0x80)
#define IPC_INT32    ((ipc_type_t)0x81)
#define IPC_STRING   ((ipc_type_t)0x82)

struct ipc_value {
  ipc_type_t type;
  union {
    gint32 v_int32;
    char *v_str;
  } value;
};

typedef guchar ipc_msg_id_t;

#define IPC_MSG_ID_FIRST             ((ipc_msg_id_t)0)
#define IPC_MSG_ID_PING              ((ipc_msg_id_t)0)
#define IPC_MSG_ID_PONG              ((ipc_msg_id_t)1)
#define IPC_MSG_ID_SCAN              ((ipc_msg_id_t)2)
#define IPC_MSG_ID_SCAN_FILE         ((ipc_msg_id_t)3)
#define IPC_MSG_ID_SCAN_END          ((ipc_msg_id_t)4)
#define IPC_MSG_ID_LAST              ((ipc_msg_id_t)4)

struct ipc_manager;

struct ipc_manager *ipc_manager_new(int input_fd, int output_fd);

void ipc_manager_free(struct ipc_manager *manager);

ipc_msg_id_t ipc_manager_get_msg_id(struct ipc_manager *manager);

int ipc_manager_get_argc(struct ipc_manager *manager);

struct ipc_value *ipc_manager_get_argv(struct ipc_manager *manager);

int ipc_manager_get_arg_at(struct ipc_manager *manager, int index, ipc_type_t type, void *pvalue);

typedef void (*ipc_handler_t)(struct ipc_manager *manager, void *data);

int ipc_manager_add_handler(struct ipc_manager *manager, ipc_msg_id_t msg_id, ipc_handler_t handler, void *data);

int ipc_manager_receive(struct ipc_manager *manager);

int ipc_manager_send_msg(struct ipc_manager *manager, ipc_msg_id_t msg_id, ...);

#endif
