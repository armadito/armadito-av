#include <libuhuru/core.h>

#include "libuhuru-config.h"

const char *uhuru_action_pretty_str(enum uhuru_action action)
{
		//fprintf(stderr, "ClamAV initialization failed: %s\n", cl_strerror(ret));
		g_log(NULL, 16, "DEBUG :: uhuru_action_pretty_str() :: %d ::\n",action);
  switch(action & (UHURU_ACTION_ALERT | UHURU_ACTION_QUARANTINE | UHURU_ACTION_REMOVE)) {
  case UHURU_ACTION_ALERT: return "alert";
  case UHURU_ACTION_ALERT | UHURU_ACTION_QUARANTINE: return "alert+quarantine";
  case UHURU_ACTION_ALERT | UHURU_ACTION_REMOVE: return "alert+removed";
  }

  return "none";
}
