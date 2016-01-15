#ifndef _UHURUJSON_H_
#define _UHURUJSON_H_

#include <json.h>

enum uhuru_json_status {
  JSON_OK = 0,
  JSON_PARSE_ERROR = 100,
  JSON_INVALID_REQUEST = 101,
  JSON_REQUEST_FAILED = 200,
};

struct uhuru_json_handler;

struct uhuru_json_handler *uhuru_json_handler_new(void);

void uhuru_json_handler_free(struct uhuru_json_handler *jh);

typedef enum uhuru_json_status (*request_fun_t)(const char *request, int id, struct json_object *params, struct uhuru *uhuru, struct json_object **p_info, const char **error_message);

enum uhuru_json_status uhuru_json_handler_process_request(struct uhuru_json_handler *jh, const char *req, int req_len, struct uhuru *uhuru, char **p_resp, int *p_resp_len);

#endif

