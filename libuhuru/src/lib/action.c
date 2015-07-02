#include <libuhuru/scan.h>

const char *uhuru_action_pretty_str(enum uhuru_action action)
{
  switch(action & (UHURU_ACTION_ALERT | UHURU_ACTION_QUARANTINE | UHURU_ACTION_REMOVE)) {
  case UHURU_ACTION_ALERT: return "alert";
  case UHURU_ACTION_ALERT | UHURU_ACTION_QUARANTINE: return "alert+quarantine";
  case UHURU_ACTION_ALERT | UHURU_ACTION_REMOVE: return "alert+removed";
  }

  return "none";
}
