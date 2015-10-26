#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "json.h"
#include "windows-service/scan.h"

json_object * create_json_obj(){

	/*Creating a json object*/
	json_object * jobj = json_object_new_object();

	/*Creating a json string*/
	json_object *jstring = json_object_new_string("Joys of Programming");

	/*Creating a json integer*/
	json_object *jint = json_object_new_int(10);

	/*Creating a json boolean*/
	json_object *jboolean = json_object_new_boolean(1);

	/*Creating a json double*/
	json_object *jdouble = json_object_new_double(2.14);

	/*Creating a json array*/
	json_object *jarray = json_object_new_array();

	/*Creating json strings*/
	json_object *jstring1 = json_object_new_string("c");
	json_object *jstring2 = json_object_new_string("c++");
	json_object *jstring3 = json_object_new_string("php");

	/*Adding the above created json strings to the array*/
	json_object_array_add(jarray, jstring1);
	json_object_array_add(jarray, jstring2);
	json_object_array_add(jarray, jstring3);

	/*Form the json object*/
	/*Each of these is like a key value pair*/
	json_object_object_add(jobj, "Site Name", jstring);
	json_object_object_add(jobj, "Technical blog", jboolean);
	json_object_object_add(jobj, "Average posts per day", jdouble);
	json_object_object_add(jobj, "Number of posts", jint);
	json_object_object_add(jobj, "Categories", jarray);

	/*Now printing the json object*/
	printf("The json object created: %s \n", json_object_to_json_string(jobj));

	return jobj;
}

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
const char* json_parse_and_process(json_object * jobj, struct new_scan* scan) {

	int scan_id = -1;
	char * server_response = "";
	const char * scan_path = "";

	json_object_object_foreach(jobj, key, val) { /*Passing through every array element*/

		if (key == NULL){
			printf("Error: uninitialized key char*\n");
			continue;
		}

		if (strcmp(key,"new_scan_id") == 0){

			if (json_object_get_type(val) != json_type_int){
				// Step 3
				return json_get_protocol_err_msg("The new_scan_id value must be an integer > 0 && < 100.");
			}

			scan_id = json_object_get_int(val);
			if (scan_id <= 0 || scan_id >= 100){ // 1 - 99 , for security reasons
				// Step 3
				return json_get_protocol_err_msg("The new_scan_id value must be an integer > 0 && < 100.");
			}

		}
		else if (strcmp(key, "scan_path") == 0 ){

			if (json_object_get_type(val) != json_type_string){
				// Step 3
				return json_get_protocol_err_msg("The path value must be a valid json string.");
			}

			scan_path = json_object_get_string(val);
		}
	}

	if (scan_id <= 0){
		// Step 3
		return json_get_protocol_err_msg("new_scan_id key was not found.");
	}

	if (strcmp(scan_path, "") == 0){
		// Step 3
		return json_get_protocol_err_msg("scan_path was empty or not found.");
	}

	scan->scan_path = scan_path;
	scan->scan_id = scan_id;

	// Step 3 : send ok to IHM
	return json_get_basic_scan_response(scan_id);
}

const char* json_get_basic_scan_response(int scan_id)
{

	json_object * jobj = json_object_new_object();
	json_object *jint = json_object_new_int(scan_id);
	json_object *jstr = json_object_new_string("ok");

	json_object_object_add(jobj, "new_scan", jstr);
	json_object_object_add(jobj, "scan_id", jint);

	return json_object_to_json_string(jobj);
}

// log error and prepare json to be sent back to IHM
const char* json_get_protocol_err_msg( const char* err_msg )
{
	printf("error:  %s\n", err_msg);

	json_object * jobj = json_object_new_object();
	json_object *jstring = json_object_new_string(err_msg);
	json_object_object_add(jobj, "error", jstring);

	return json_object_to_json_string(jobj);
}

