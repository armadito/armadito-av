#ifndef LIBJRPC_TEST_H
#define LIBJRPC_TEST_H

struct operands {
	int op1;
	int op2;
	int result;
};

#define MARSHALL_DECLARATIONS
#include "test-marshall.h"

#define UNMARSHALL_DECLARATIONS
#include "test-marshall.h"

#endif
