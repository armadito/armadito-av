#ifndef _SCAN_H_
#define _SCAN_H_

#include "jsonhandler_p.h"

enum uhuru_json_status scan_response_cb(struct uhuru *uhuru, struct json_request *req, struct json_response *resp, void **request_data);

void scan_process_cb(struct uhuru *uhuru, void *request_data);

#endif
