#ifndef _STATE_H_
#define _STATE_H_

#include "jsonhandlerp.h"

enum a6o_json_status state_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data);

#endif
