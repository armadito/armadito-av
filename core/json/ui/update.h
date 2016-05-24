#ifndef __UI_UPDATE_H__
#define __UI_UPDATE_H__

#include "jsonhandlerp.h"


#define DB_DESC_URL "http://db.armadito.org/current/armaditodbvirus.json"
#define DB_SIG_URL	"http://db.armadito.org/current/armaditodbvirus.json.sig"



char * get_db_module_path(char * filename, char * module);
int update_modules_db(struct armadito * armadito);

enum a6o_json_status update_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data);

#endif
