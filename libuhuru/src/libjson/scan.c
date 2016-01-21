#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "uhurujson.h"
#include "scan.h"

#include <json.h>
#include <stdlib.h>
#include <string.h>

enum uhuru_json_status scan_request_cb(const char *request, int id, struct json_object *params, struct uhuru *uhuru, struct json_object **p_info, const char **p_error_message)
{
#ifdef DEBUG
  uhuru_log(UHURU_LOG_SERVICE, UHURU_LOG_LEVEL_DEBUG, "JSON: scan cb called");
#endif

  *p_info = NULL;

  return JSON_OK;
}

