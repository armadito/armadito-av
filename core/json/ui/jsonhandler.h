#ifndef _JSONHANDLER_H_
#define _JSONHANDLER_H_

#include <libarmadito.h>

#include <json.h>

enum a6o_json_status {
	JSON_OK = 0,
	JSON_PARSE_ERROR = 100,
	JSON_MALFORMED_REQUEST = 101,
	JSON_INVALID_REQUEST = 102,
	JSON_REQUEST_FAILED = 200,
	JSON_UNEXPECTED_ERR = 300,
};

struct a6o_json_handler;

struct a6o_json_handler *a6o_json_handler_new(struct armadito *armadito);

void a6o_json_handler_free(struct a6o_json_handler *jh);

enum a6o_json_status a6o_json_handler_get_response(struct a6o_json_handler *jh, const char *req, int req_len, char **p_resp, int *p_resp_len);

void a6o_json_handler_process(struct a6o_json_handler *j);

#endif

