#ifndef _STATE_H_
#define _STATE_H_

#include "jsonhandler_p.h"

enum uhuru_json_status state_response_cb(struct uhuru *uhuru, struct json_request *req, struct json_response *resp, void **request_data);

#endif
