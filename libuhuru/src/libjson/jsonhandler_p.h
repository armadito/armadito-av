#ifndef _JSONP_H_
#define _JSONP_H_

#include "jsonhandler.h"

#include <assert.h>
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

typedef enum uhuru_json_status (*request_cb_t)(struct uhuru *uhuru, struct json_request *req, struct json_response *resp);

static inline void json_request_destroy(struct json_request *req)
{
  if (req->request != NULL)
    free((void *)req->request);

  if (req->params != NULL)
    assert(json_object_put(req->params));
}

static inline void json_response_destroy(struct json_response *resp)
{
  if (resp->response != NULL)
    free((void *)resp->response);

  if (resp->info != NULL)
    assert(json_object_put(resp->info));

  if (resp->error_message != NULL)
    free((void *)resp->error_message);
}

#endif
