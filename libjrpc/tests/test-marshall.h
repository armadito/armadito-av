#include <libjrpc/marshall-gen.h>

/* for testing */
JRPC_DEFINE_STRUCT(operands)
	JRPC_DEFINE_FIELD_INT(int, op1)
	JRPC_DEFINE_FIELD_INT(int, op2)
	JRPC_DEFINE_FIELD_INT(int, result)
JRPC_END_STRUCT
