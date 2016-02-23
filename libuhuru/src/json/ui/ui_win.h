#include "jsonhandler.h"

// AV service request to IHM.
enum uhuru_json_status json_handler_ui_request(const char * ip_path, const char * request, int request_len, char * response, int response_len);