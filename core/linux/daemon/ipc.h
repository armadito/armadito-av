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

#ifndef _IPC_H_
#define _IPC_H_

typedef int ipc_int32_t;

typedef unsigned char ipc_type_t;

#define IPC_NONE_T     ((ipc_type_t)0x80)
#define IPC_INT32_T    ((ipc_type_t)0x81)
#define IPC_STRING_T   ((ipc_type_t)0x82)

struct ipc_value {
	ipc_type_t type;
	union {
		ipc_int32_t v_int32;
		char *v_str;
	} value;
};

typedef unsigned char ipc_msg_id_t;

#define IPC_MSG_ID_FIRST             ((ipc_msg_id_t)0)
#define IPC_MSG_ID_PING              ((ipc_msg_id_t)0)
#define IPC_MSG_ID_PONG              ((ipc_msg_id_t)1)
#define IPC_MSG_ID_SCAN              ((ipc_msg_id_t)2)
#define IPC_MSG_ID_SCAN_FILE         ((ipc_msg_id_t)3)
#define IPC_MSG_ID_SCAN_END          ((ipc_msg_id_t)4)
#define IPC_MSG_ID_INFO              ((ipc_msg_id_t)5)
#define IPC_MSG_ID_INFO_MODULE       ((ipc_msg_id_t)6)
#define IPC_MSG_ID_INFO_END          ((ipc_msg_id_t)7)
#define IPC_MSG_ID_LAST              ((ipc_msg_id_t)7)

struct ipc_manager;

struct ipc_manager *ipc_manager_new(int io_fd);

void ipc_manager_free(struct ipc_manager *manager);

ipc_msg_id_t ipc_manager_get_msg_id(struct ipc_manager *manager);

int ipc_manager_get_argc(struct ipc_manager *manager);

struct ipc_value *ipc_manager_get_argv(struct ipc_manager *manager);

int ipc_manager_get_arg_at(struct ipc_manager *manager, int index, ipc_type_t type, void *pvalue);

typedef void (*ipc_handler_t)(struct ipc_manager *manager, void *data);

int ipc_manager_add_handler(struct ipc_manager *manager, ipc_msg_id_t msg_id, ipc_handler_t handler, void *data);

int ipc_manager_receive(struct ipc_manager *manager);

int ipc_manager_msg_send(struct ipc_manager *manager, ipc_msg_id_t msg_id, ...);

int ipc_manager_msg_begin(struct ipc_manager *manager, ipc_msg_id_t msg_id);
int ipc_manager_msg_add(struct ipc_manager *manager, ...);
int ipc_manager_msg_end(struct ipc_manager *manager);

#endif
