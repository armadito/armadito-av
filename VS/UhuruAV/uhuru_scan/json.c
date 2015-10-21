#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "json.h"


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

void json_parse_array(json_object *jobj, char *key) {
	void json_parse(json_object * jobj); /*Forward Declaration*/
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
			json_parse_array(jvalue, NULL);
		}
		else if (type != json_type_object) {
			printf("value[%d]: ", i);
			print_json_value(jvalue);
		}
		else {
			json_parse(jvalue);
		}
	}
}

/*Parsing the json object*/
void json_parse(json_object * jobj) {

	json_object * jfound;
	enum json_type type;

	json_object_object_foreach(jobj, key, val) { /*Passing through every array element*/

		if (key == NULL){
			printf("Error: uninitialized key char*\n");
			continue;
		}
		type = json_object_get_type(val);
		printf("type: ", type);

		switch (type) {
		case json_type_boolean:
		case json_type_double:
		case json_type_int:
		case json_type_string: print_json_value(val);
			break;
		case json_type_object: printf("json_type_object \n");
			json_object_object_get_ex(jobj, key, &jfound);
			json_parse(jfound);
			break;
		case json_type_array: printf("type: json_type_array, ");
			json_parse_array(jobj, key);
			break;
		}
	}
}
