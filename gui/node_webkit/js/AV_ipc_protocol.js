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

var IPC_MSG_ID_FIRST = 0;
var IPC_MSG_ID_PING = 0;
var IPC_MSG_ID_PONG = 1;
var IPC_MSG_ID_SCAN = 2;
var IPC_MSG_ID_SCAN_FILE = 3;
var IPC_MSG_ID_SCAN_END = 4;
var IPC_MSG_ID_INFO = 5;
var IPC_MSG_ID_INFO_MODULE = 6;
var IPC_MSG_ID_INFO_END = 7;
var IPC_MSG_ID_LAST = 7;

var IPC_NONE_T = 0x80;
var IPC_INT32_T = 0x81;
var IPC_STRING_T = 0x82;

function parse_message ( message )
{
	// TODO
	
/* C Hardcore implementation
static void ipc_manager_input_char(struct ipc_manager *m, guchar c)
{
  switch(m->state) {
  case EXPECTING_MSG_ID:
    m->msg_id = c;
    m->state = EXPECTING_ARG;
    break;
  case EXPECTING_ARG:
    switch(c) {
    case IPC_INT32_T:
      m->received_count = 0;
      m->state = IN_ARG_INT32;
      break;
    case IPC_STRING_T:
      m->state = IN_ARG_STRING;
      break;
    case IPC_NONE_T:
      m->state = EXPECTING_MSG_ID;
      ipc_manager_end_of_msg(m);
      break;
    default:
      g_log(NULL, G_LOG_LEVEL_ERROR, "error in ipc_manager_receive: invalid type msg_id %c %d", c, c);
      break;
    }
    break;
  case IN_ARG_INT32:
    m->int32_arg.t[m->received_count++] = c;
    if (m->received_count == sizeof(gint32)) {
      ipc_manager_add_int32_arg(m);
      m->received_count = 0;
      m->state = EXPECTING_ARG;
    }
    break;
  case IN_ARG_STRING:
    if (c != '\0')
      g_string_append_c(m->str_arg, c);
    else {
      ipc_manager_add_str_arg(m);
      g_string_truncate(m->str_arg, 0);
      m->state = EXPECTING_ARG;
    }
    break;
  }
 */
  
}

function create_message ( msg_id )
{
	// TODO
	var message = "Hey Jude";
	return message;
}


