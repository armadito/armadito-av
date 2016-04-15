#ifndef __UI_UPDATE_H__
#define __UI_UPDATE_H__

#include "jsonhandlerp.h"


#define DB_DESC_URL "http://uhuru.gendarmerie.fr/current/uhurudbvirus.json"
#define DB_SIG_URL "http://uhuru.gendarmerie.fr/current/uhurudbvirus.json.sig"
#define DB_CACHE_PATH "modules\\DB\\dbcache"



int update_modules_db(struct armadito * armadito);

enum a6o_json_status update_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data);

#endif