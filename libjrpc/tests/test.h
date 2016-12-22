#ifndef LIBJRPC_TEST_H
#define LIBJRPC_TEST_H

struct cplx {
	int re;
	int im;
};

enum op_type {
	OP_INT,
	OP_CPLX,
	OP_VCPLX,
};

struct operands {
	enum op_type opt;
	/* integer */
	int i_op1;
	int i_op2;
	int i_result;
	/* complex */
	struct cplx *c_op1;
	struct cplx *c_op2;
	struct cplx *c_result;
	/* vector of complex */
	struct cplx **v_op1;
	struct cplx **v_op2;
	struct cplx **v_result;
};

struct notify_action {
	const char *whot;
};

#define MARSHALL_DECLARATIONS
#include "test-marshall.h"

#define UNMARSHALL_DECLARATIONS
#include "test-marshall.h"

struct operands *operands_new(int allow_null);

#endif
