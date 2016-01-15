#ifndef _STATE_H_
#define _STATE_H_

enum uhuru_json_status state_request(const char *request, int id, struct json_object *params, struct uhuru *uhuru, struct json_object **p_info, const char **error_message);

#endif
