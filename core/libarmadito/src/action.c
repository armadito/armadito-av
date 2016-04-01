#include <libarmadito.h>

#include "libarmadito-config.h"

const char *a6o_action_pretty_str(enum a6o_action action)
{
	switch(action & (ARMADITO_ACTION_ALERT | ARMADITO_ACTION_QUARANTINE | ARMADITO_ACTION_REMOVE)) {
	case ARMADITO_ACTION_ALERT:
		return "alert";
	case ARMADITO_ACTION_ALERT | ARMADITO_ACTION_QUARANTINE:
		return "alert+quarantine";
	case ARMADITO_ACTION_ALERT | ARMADITO_ACTION_REMOVE:
		return "alert+removed";
	}

	return "none";
}
