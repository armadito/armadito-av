#ifndef _SCAN_H_
#define _SCAN_H_

#include "jsonhandler_p.h"

enum uhuru_json_status scan_request_cb(struct uhuru *uhuru, struct json_request *req, struct json_response *resp);

#endif
