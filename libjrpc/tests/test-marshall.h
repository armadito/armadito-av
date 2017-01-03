#include <libjrpc/marshall.h>

/* for testing */

JRPC_STRUCT(cplx)
	JRPC_FIELD_INT(int, re)
	JRPC_FIELD_INT(int, im)
JRPC_END_STRUCT

JRPC_ENUM(op_type)
	JRPC_ENUM_VALUE(OP_INT)
	JRPC_ENUM_VALUE(OP_CPLX)
JRPC_END_ENUM

JRPC_STRUCT(operands)
	JRPC_FIELD_ENUM(op_type, opt)
	JRPC_FIELD_INT(int, i_op1)
	JRPC_FIELD_INT(int, i_op2)
	JRPC_FIELD_INT(int, i_result)
	JRPC_FIELD_STRUCT(cplx, c_op1)
	JRPC_FIELD_STRUCT(cplx, c_op2)
	JRPC_FIELD_STRUCT(cplx, c_result)
	JRPC_FIELD_ARRAY(cplx, v_op1)
	JRPC_FIELD_ARRAY(cplx, v_op2)
	JRPC_FIELD_ARRAY(cplx, v_result)
JRPC_END_STRUCT

JRPC_STRUCT(notify_action)
	JRPC_FIELD_STRING(whot)
JRPC_END_STRUCT
