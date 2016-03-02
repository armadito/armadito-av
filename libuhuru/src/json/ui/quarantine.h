#ifndef _QUARANTINE_H_
#define _QUARANTINE_H_

#include "jsonhandler_p.h"

enum uhuru_json_status quarantine_response_cb(struct uhuru *uhuru, struct json_request *req, struct json_response *resp, void **request_data);

#endif
