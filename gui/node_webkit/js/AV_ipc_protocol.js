/*

Here is described the current used protocol used to communicate with Uhuru-AV
Please update this if protocol change.

id # (tag_type # char* ou int32 #) X fois
----------

id :
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

tag_type : 
#define IPC_NONE_T     ((ipc_type_t)0x80)
#define IPC_INT32_T    ((ipc_type_t)0x81)
#define IPC_STRING_T   ((ipc_type_t)0x82)

*/

function parse_message ( message )
{
	// TODO
}



