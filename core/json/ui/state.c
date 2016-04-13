#include <libarmadito.h>
#include "config/libarmadito-config.h"

#include "jsonhandler.h"
#include "state.h"
#include "os/string.h"

#include <json.h>
#include <stdlib.h>
#include <string.h>

static struct json_object *antivirus_json(struct a6o_info *info)
{
	struct json_object *j_antivirus;

	j_antivirus = json_object_new_object();

	json_object_object_add(j_antivirus, "version", json_object_new_string("3.14"));
	json_object_object_add(j_antivirus, "service", json_object_new_string("on"));
	json_object_object_add(j_antivirus, "real-time-protection", json_object_new_string("on"));

	return j_antivirus;
}

static struct json_object *update_status_json(enum a6o_update_status status)
{
	switch(status) {
	case ARMADITO_UPDATE_OK:
		return json_object_new_string("up-to-date");
	case ARMADITO_UPDATE_LATE:
		return json_object_new_string("late");
	case ARMADITO_UPDATE_CRITICAL:
		return json_object_new_string("critical");
	case ARMADITO_UPDATE_NON_AVAILABLE:
		return json_object_new_string("non-available");
	}

	return json_object_new_string("unknown");
}

static struct json_object *update_json(enum a6o_update_status status, const char *update_date)
{
	struct json_object *j_update;

	j_update = json_object_new_object();

	json_object_object_add(j_update, "status", update_status_json(status));
	if (update_date != NULL)
		json_object_object_add(j_update, "last-update", json_object_new_string(update_date));

	return j_update;
}

static struct json_object *state_json(struct a6o_info *info)
{
	struct json_object *j_state, *j_mod_array;
	struct a6o_module_info **m;

	j_state = json_object_new_object();

	json_object_object_add(j_state, "antivirus", antivirus_json(info));
	json_object_object_add(j_state, "update", update_json(info->global_status, "1970-01-01 00:00"));

	j_mod_array = json_object_new_array();

	for(m = info->module_infos; *m != NULL; m++) {
		struct json_object *j_mod = json_object_new_object();

		json_object_object_add(j_mod, "name", json_object_new_string((*m)->name));
		json_object_object_add(j_mod, "version", json_object_new_string("0.0.0"));
		json_object_object_add(j_mod, "update", update_json((*m)->mod_status, (*m)->update_date));

		json_object_array_add(j_mod_array, j_mod);
	}

	json_object_object_add(j_state, "modules", j_mod_array);

	return j_state;
}

enum a6o_json_status state_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data)
{
	struct a6o_info *info;

#ifdef DEBUG
	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "JSON: state cb called");
#endif

	info = a6o_info_new(armadito);

	if (info == NULL) {
		resp->error_message = os_strdup("getting info failed");

		return JSON_REQUEST_FAILED;
	}

	resp->info = state_json(info);

	a6o_info_free(info);

	return JSON_OK;
}

