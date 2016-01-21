#ifndef _SCAN_H_
#define _SCAN_H_

enum uhuru_json_status scan_request_cb(const char *request, int id, struct json_object *params, struct uhuru *uhuru, struct json_object **p_info, const char **error_message);

#endif
