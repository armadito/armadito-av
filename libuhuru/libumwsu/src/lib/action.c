#include <libumwsu/scan.h>

const char *umwsu_action_pretty_str(enum umwsu_action action)
{
  switch(action & (UMWSU_ACTION_ALERT | UMWSU_ACTION_QUARANTINE | UMWSU_ACTION_REMOVE)) {
  case UMWSU_ACTION_ALERT: return "alert";
  case UMWSU_ACTION_ALERT | UMWSU_ACTION_QUARANTINE: return "alert+quarantine";
  case UMWSU_ACTION_ALERT | UMWSU_ACTION_REMOVE: return "alert+removed";
  }

  return "none";
}
