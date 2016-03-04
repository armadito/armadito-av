#ifndef _JSONHANDLER_H_
#define _JSONHANDLER_H_

#include <json.h>
#include <libuhuru/core.h>

enum uhuru_json_status {
  JSON_OK = 0,
  JSON_PARSE_ERROR = 100,
  JSON_MALFORMED_REQUEST = 101,
  JSON_INVALID_REQUEST = 102,
  JSON_REQUEST_FAILED = 200,
  JSON_UNEXPECTED_ERR = 300,
};

struct uhuru_json_handler;

struct uhuru_json_handler *uhuru_json_handler_new(struct uhuru *uhuru);

void uhuru_json_handler_free(struct uhuru_json_handler *jh);

enum uhuru_json_status uhuru_json_handler_get_response(struct uhuru_json_handler *jh, const char *req, int req_len, char **p_resp, int *p_resp_len);

void uhuru_json_handler_process(struct uhuru_json_handler *j);

#endif

