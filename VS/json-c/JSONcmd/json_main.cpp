// JSONcmd.cpp : Defines the entry point for the console application.
//

// #include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "json_util.h"
#include "json.h"

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

int main(int argc, char* argv[])
{
	printf("########JSON######## \n");

	char * string = "{\"sitename\" : \"joys of programming\",\"categories\" :[\"c\", [\"c++\", \"c\"], \"java\", \"PHP\"],\"author-details\" : { \"admin\": false, \"name\" : \"Joys of Programming\", \"Number of Posts\" : 10 }}";

	printf("JSON string: %s\n", string);

	json_object * jobj = json_tokener_parse(string);
	json_parse(jobj);

	printf("Parsing OK !\n", string);

	system("pause");
	return 0;
}

