#include "test.h"

#define MARSHALL_FUNCTIONS
#include "test-marshall.h"

#define UNMARSHALL_FUNCTIONS
#include "test-marshall.h"

struct operands *operands_new(int allow_null)
{
	struct operands *op = calloc(1, sizeof(struct operands));

	if (!allow_null) {
		op->c_op1 = calloc(1, sizeof(struct cplx));
		op->c_op2 = calloc(1, sizeof(struct cplx));
		op->c_result = calloc(1, sizeof(struct cplx));

		op->v_op1 = calloc(1, sizeof(struct cplx *));
		op->v_op2 = calloc(1, sizeof(struct cplx *));
		op->v_result = calloc(1, sizeof(struct cplx *));
	}

	return op;
}

