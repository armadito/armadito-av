#ifndef _LIBUHURU_COMMON_ACTION_H_
#define _LIBUHURU_COMMON_ACTION_H_

enum uhuru_action {
  UHURU_ACTION_NONE         = 0,
  UHURU_ACTION_ALERT        = 1 << 1,
  UHURU_ACTION_QUARANTINE   = 1 << 2,
  UHURU_ACTION_REMOVE       = 1 << 3,
};

#endif
