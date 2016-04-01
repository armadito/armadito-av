#ifndef _LIBARMADITO_ACTION_H_
#define _LIBARMADITO_ACTION_H_

enum a6o_action {
	ARMADITO_ACTION_NONE         = 0,
	ARMADITO_ACTION_ALERT        = 1 << 1,
	ARMADITO_ACTION_QUARANTINE   = 1 << 2,
	ARMADITO_ACTION_REMOVE       = 1 << 3,
};

const char *a6o_action_pretty_str(enum a6o_action action);

#endif
