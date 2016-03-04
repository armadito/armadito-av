#include "libuhuru-config.h"

#include "quarantine.h"

#include <json.h>

enum uhuru_json_status quarantine_response_cb(struct uhuru *uhuru, struct json_request *req, struct json_response *resp, void **request_data)
{
  enum uhuru_json_status status = JSON_OK;

  return status;
}
