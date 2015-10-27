#ifndef _LIBUHURU_COMMON_ACTION_H_
#define _LIBUHURU_COMMON_ACTION_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum uhuru_action {
  UHURU_ACTION_NONE         = 0,
  UHURU_ACTION_ALERT        = 1 << 1,
  UHURU_ACTION_QUARANTINE   = 1 << 2,
  UHURU_ACTION_REMOVE       = 1 << 3,
};

const char *uhuru_action_pretty_str(enum uhuru_action action);

#ifdef __cplusplus
}
#endif


#endif
