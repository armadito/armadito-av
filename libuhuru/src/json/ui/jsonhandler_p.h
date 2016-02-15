#ifndef _JSONHANDLER_P_H_
#define _JSONHANDLER_P_H_

#include "jsonhandler.h"

#include <json.h>
#include <stdlib.h>

struct json_request {
  const char *request;
  int id;
  struct json_object *params;
};

struct json_response {
  const char *response;
  int id;
  enum uhuru_json_status status;
  struct json_object *info;
  const char *error_message;
};

typedef enum uhuru_json_status (*response_cb_t)(struct uhuru *uhuru, struct json_request *req, struct json_response *resp, void **request_data);

typedef void (*process_cb_t)(struct uhuru *uhuru, void *request_data);

#endif
