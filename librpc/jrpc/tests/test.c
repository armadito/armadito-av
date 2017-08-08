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

void operands_free(struct operands *op)
{
	if (op->c_op1 != NULL) {
		free(op->c_op1);
		free(op->c_op2);
		free(op->c_result);

		free(op->v_op1);
		free(op->v_op2);
		free(op->v_result);
	}
	free(op);
}
