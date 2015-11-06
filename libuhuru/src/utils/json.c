#include <libuhuru/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "json.h"
#include "windows-service/scan.h"

/*printing the value corresponding to boolean, double, integer and strings*/
void print_json_value(json_object *jobj){
	enum json_type type;

	type = json_object_get_type(jobj); /*Getting the type of the json object*/
	printf("type: ", type);

	switch (type) {
	case json_type_boolean: printf("json_type_boolean\n");
		printf("value: %s\n", json_object_get_boolean(jobj) ? "true" : "false");
		break;
	case json_type_double: printf("json_type_double\n");
		printf("          value: %lf\n", json_object_get_double(jobj));
		break;
	case json_type_int: printf("json_type_int\n");
		printf("          value: %d\n", json_object_get_int(jobj));
		break;
	case json_type_string: printf("json_type_string\n");
		printf("          value: %s\n", json_object_get_string(jobj));
		break;
	}

}

void json_parse_array_and_print(json_object *jobj, char *key) {
	void json_parse_and_print(json_object * jobj); /*Forward Declaration*/
	enum json_type type;

	json_object *jarray = jobj; /*Simply get the array*/
	if (key) {
		json_object_object_get_ex(jobj, key, &jarray); /*Getting the array if it is a key value pair*/
	}

	int arraylen = json_object_array_length(jarray); /*Getting the length of the array*/
	printf("Array Length: %d\n", arraylen);
	int i;
	json_object * jvalue;

	for (i = 0; i< arraylen; i++){
		jvalue = json_object_array_get_idx(jarray, i); /*Getting the array element at position i*/
		type = json_object_get_type(jvalue);
		if (type == json_type_array) {
			json_parse_array_and_print(jvalue, NULL);
		}
		else if (type != json_type_object) {
			printf("value[%d]: ", i);
			print_json_value(jvalue);
		}
		else {
			json_parse_and_print(jvalue);
		}
	}
}

/*Parsing the json object*/
void json_parse_and_print(json_object * jobj) {

	json_object * jfound;
	enum json_type type;


	json_object_object_foreach(jobj, key, val) { /*Passing through every array element*/

		if (key == NULL){
			printf("Error: uninitialized key char*\n");
			continue;
		}

		if (val == NULL){
			printf("Error: uninitialized val object\n");
			continue;
		}

		printf(" %s : ", key);
		type = json_object_get_type(val);

		switch (type) {
		case json_type_boolean:
		case json_type_double:
		case json_type_int:
		case json_type_string: print_json_value(val);
			break;
		case json_type_object: printf("json_type_object \n");
			json_object_object_get_ex(jobj, key, &jfound);
			json_parse_and_print(jfound);
			break;
		case json_type_array: printf("type: json_type_array, ");
			json_parse_array_and_print(jobj, key);
			break;
		}
	}
}

/*Parsing the json object*/
const char* json_parse_and_process(json_object * jobj, struct new_scan_action* scan) {

	int scan_id = -1;
	char * server_response = "";
	const char * scan_path = "";
	const char * scan_action = "";

	json_object_object_foreach(jobj, key, val) { /*Passing through every array element*/

		if (key == NULL){
			printf("Error: uninitialized key char*\n");
			continue;
		}

		if (strcmp(key, "scan_id") == 0){ 

			if (json_object_get_type(val) != json_type_int){
				return json_get_protocol_err_msg("The scan_id value must be an integer > 0 && < 100.", 101);
			}

			scan_id = json_object_get_int(val);
			if (scan_id <= 0 || scan_id >= 100){ // 1 - 99 , for security reasons
				return json_get_protocol_err_msg("The scan_id value must be an integer > 0 && < 100.", 101);
			}
		}
		else if (strcmp(key, "scan_path") == 0 ){

			if (json_object_get_type(val) != json_type_string){
				// Step 3
				return json_get_protocol_err_msg("The scan_path value must be a valid json string.", 101);
			}

			scan_path = json_object_get_string(val);
		}
		else if (strcmp(key, "scan_action") == 0){

			if (json_object_get_type(val) != json_type_string){
				// Step 3
				return json_get_protocol_err_msg("The scan_action value must be a valid json string.", 101);
			}

			scan_action = json_object_get_string(val);
		}
	}

	if (scan_id <= 0){
		// Step 3
		return json_get_protocol_err_msg("new_scan_id key was not found.", 101);
	}

	if (strcmp(scan_action, "") == 0){
		// Step 3
		return json_get_protocol_err_msg("scan_action was empty or not found.", scan_id);
	}

	// scan_path = "" if scan_action != new_scan
	if (strcmp(scan_path, "") == 0 && strcmp(scan_action, "new_scan") == 0){
		// Step 3
		return json_get_protocol_err_msg("scan_path was empty or not found.", scan_id);
	}

	scan->scan_path = scan_path;
	scan->scan_id = scan_id;
	scan->scan_action = scan_action;

	// Step 3 : send ok to IHM
	return json_get_basic_ok_response(scan_id, scan_action);
}

const char* json_get_basic_ok_response(int scan_id, const char* scan_action)
{

	json_object * jobj = json_object_new_object();
	json_object *jint = json_object_new_int(scan_id);
	json_object *jstr = json_object_new_string("ok");

	json_object_object_add(jobj, scan_action, jstr);
	json_object_object_add(jobj, "scan_id", jint);

	return json_object_to_json_string(jobj);
}

// log error and prepare json to be sent back to IHM
// scan_id = 101 means error with unknown scan_id value.
const char* json_get_protocol_err_msg( const char* err_msg, int scan_id )
{
	printf("error:  %s\n", err_msg);

	json_object * jobj = json_object_new_object();
	json_object *jint = json_object_new_int(scan_id);
	json_object *jstring = json_object_new_string(err_msg);

	json_object_object_add(jobj, "error", jstring);
	json_object_object_add(jobj, "scan_id", jint);

	return json_object_to_json_string(jobj);
}

// uhuru_report* to JSON
const char* json_get_report_msg(uhuru_report* report){

	const char* report_status;
	const char* report_action;

	// In order to send "null" string in JSON.
	if (report->mod_name == NULL){report->mod_name = "null";}
	if (report->mod_report == NULL){report->mod_report = "null";}
	if (report->path == NULL){report->path = "null";}

	report_status = uhuru_file_status_pretty_str(report->status);
	report_action = uhuru_action_pretty_str(report->action);

	json_object * jobj = json_object_new_object();
	json_object *jint = json_object_new_int(report->scan_id);
	json_object *jint2 = json_object_new_int(report->progress);
	json_object *jstring1 = json_object_new_string(report_status);
	json_object *jstring2 = json_object_new_string(report->path);
	json_object *jstring4 = json_object_new_string(report_action);
	json_object *jstring5 = json_object_new_string(report->mod_name);
	json_object *jstring6 = json_object_new_string(report->mod_report);

	// add order is kept
	json_object_object_add(jobj, "scan_progress", jint2);
	json_object_object_add(jobj, "scan_id", jint);
	json_object_object_add(jobj, "scan_file_path", jstring2);
	json_object_object_add(jobj, "scan_status", jstring1);
	json_object_object_add(jobj, "scan_action", jstring4);
	json_object_object_add(jobj, "mod_name", jstring5);
	json_object_object_add(jobj, "mod_report", jstring6);

	// NULL in order that free is not called
	if (report->mod_name == "null"){ report->mod_name = NULL; }
	if (report->mod_report == "null"){ report->mod_report = NULL; }
	if (report->path == "null"){ report->path = NULL; }

	return json_object_to_json_string(jobj);
}
