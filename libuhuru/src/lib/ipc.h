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

typedef guchar ipc_tag_t;

struct ipc_manager;

struct ipc_manager *ipc_manager_new(int input_fd, int output_fd);

void ipc_manager_free(struct ipc_manager *manager);

int ipc_manager_get_argc(struct ipc_manager *manager);

struct ipc_value *ipc_manager_get_argv(struct ipc_manager *manager);

typedef void (*ipc_handler_t)(struct ipc_manager *manager, void *data);

int ipc_manager_add_callback(struct ipc_manager *manager, ipc_tag_t tag, ipc_handler_t cb, void *data);

int ipc_manager_receive(struct ipc_manager *manager);

int ipc_manager_send_msg(struct ipc_manager *manager, ipc_tag_t tag, ...);

#endif
