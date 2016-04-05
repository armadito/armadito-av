#ifndef _QUARANTINE_H_
#define _QUARANTINE_H_

#include "jsonhandlerp.h"

enum a6o_json_status quarantine_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data);

#endif
