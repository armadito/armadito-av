#ifndef _SCAN_H_
#define _SCAN_H_

#include "jsonhandlerp.h"

enum a6o_json_status scan_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data);

void scan_process_cb(struct armadito *armadito, void *request_data);

enum a6o_json_status scan_cancel_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data);

#endif
