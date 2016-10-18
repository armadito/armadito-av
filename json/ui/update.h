/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef __UI_UPDATE_H__
#define __UI_UPDATE_H__

#include "jsonhandlerp.h"


#define DB_DESC_URL "http://db.armadito.org/current/armaditodbvirus.json"
#define DB_SIG_URL	"http://db.armadito.org/current/armaditodbvirus.json.sig"



char * get_db_module_path(char * filename, char * module);
int update_modules_db(struct armadito * armadito);

enum a6o_json_status update_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data);

#endif
