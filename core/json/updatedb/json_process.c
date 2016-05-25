/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "json_process.h"



int fillPackageParam(Package** pkgList,int index, char * key, char * value) {

	if (key == NULL || value == NULL || index < 0) {
		printf("[-] Error :: fillPackageParam :: Invalid parameter\n");
		return -1;
	}
	
	if (strncmp(key,"displayName",11) == 0) {

		pkgList[index]->displayname = value;

	}
	else if (strncmp(key,"fileUrl",7) == 0) {

		pkgList[index]->fileurl = value;
		//printf("index = %d :: value = %s\n", index,value);
		//printf("index = %d :: value = %s\n", index,pkgList[index]->fileurl);
		//pkgList[index]->fileurl = _strdup(value);
		//printf("index = %d :: value = %s\n", index,value);

	}
	else if (strncmp(key,"controlSum",10) == 0) {

		pkgList[index]->controlsum = value;

	}
	else if (strncmp(key,"controlType",11) == 0) {

		pkgList[index]->controltype = value;

	}
	else if (strncmp(key,"licence",7) == 0) {

		pkgList[index]->licence = value;

	}
	else {
		printf("[-] Error :: fillPackageParam :: Invalid key :: %s !\n",key);
		return -2;
	}

	return 0;

}


/*
	This function parses json object and  fills the array of packages structure.
*/
int json_parse_obj_and_process(json_object * obj, int index, Package** pkgList) {

	int ret = 0;
	char *key = NULL;
	struct json_object *val = NULL;
	struct lh_entry *entrykey = NULL;
	struct lh_entry *entry_nextkey = NULL;

	enum json_type type;
	char * string = NULL;

	if (obj == NULL || pkgList == NULL) {
		printf("[-] Error :: json_parse_obj_and_process :: Invalid parameter\n");
		return -1;
	}


	for (entrykey = json_object_get_object(obj)->head;
		 (entrykey ? (key = (char*)entrykey->k, val = (struct json_object*)entrykey->v, entry_nextkey = entrykey->next, entrykey) : 0);
		 entrykey = entry_nextkey) {

		if (key == NULL) {
			printf("[-] Error :: json_parse_obj_and_process :: uninitialized key char*\n");
			continue;
		}

		//printf("[+] Debug :: json_parse_obj_and_process :: key = %s\n", key);

		type = json_object_get_type(val);

		switch (type) {

			case json_type_boolean:				
				break;
			case json_type_double:
				break;
			case json_type_int:
				break;
			case json_type_string:				
				string = json_object_get_string(val);
				//printf("[+] Debug :: json_parse_obj_and_process :: string value = %s\n",string);
				// Fill the package structure.
				if (fillPackageParam(pkgList, index, key, string) < 0) {
					printf(" [-] Error :: json_parse_obj_and_process :: Fill package param failed :: id = %d, key = %s, string = %s\n",index, key,string);
					ret = -2;
					return ret;
				}
				break;
			case json_type_object:
				//json_parse_obj_rec(val); // not supposed to happend
				break;
			case json_type_array:
				//json_parse_array(val); // not supposed to happend
				break;
			default:
				printf(" [-] Error :: json_parse_obj_and_process :: Unknown json_object type :: %d \n",type);
				break;

		}

	}


	return ret;

}


/*
	This function parses a array json object and fills the package list struct.
	Returns the number of packages.
*/
int json_parse_array(json_object * jarray, Package ** pkgList) {

	int ret = 0;
	int array_len = 0;
	int i = 0;
	json_object * jvalue;
	enum json_type type;

	
	// 
	if (jarray == NULL) {
		printf("[-] Error :: json_parse_array :: Invalid parameter\n");
		return -1;
	}

	array_len = json_object_array_length(jarray);

	if (array_len <= 0) {
		printf("[-] Error :: json_parse_array :: Empty array!\n");
		return -2;
	}


	//printf("[+] Debug :: json_parse_array :: package %d ::\n",i);

	for (i = 0; i < array_len; i++) {

		//printf("[+] Debug :: json_parse_array :: package %d ::\n",i);
		jvalue = json_object_array_get_idx(jarray, i) ;
		type = json_object_get_type(jvalue);

		switch (type) {
			case json_type_boolean:
				break;
			case json_type_double:
				break;
			case json_type_int:
				break;
			case json_type_string:
				break;
			case json_type_object:
				json_parse_obj_and_process(jvalue,i,pkgList);
				break;
			case json_type_array:
				break;
			default:
				printf("[-] Warning :: json_parse_array :: Unknown json_object type :: %d \n",type);
				break;
		}

	}

	//PrintPackageList(pkgList, array_len);	

	return array_len;
}


/*
	This function parses an json object (description file)
	And fills the pkgList.
	Returns the number of packages on success
	or an error code (<0) on error.
*/
int json_parse_obj_rec(json_object * obj, Package ** pkgList) {

	int ret = 0;
	char *key = NULL;
	struct json_object *val = NULL;
	//struct json_object *pkgList = NULL;
	struct lh_entry *entrykey = NULL;
	struct lh_entry *entry_nextkey = NULL;
	int array_len = 0;
	int i = 0;

	enum json_type type;

	if (obj == NULL) {
		printf("[-] Error :: json_parse_obj_rec :: Invalid parameter\n");
		return -1;
	}

	for (entrykey = json_object_get_object(obj)->head;
		 (entrykey ? (key = (char*)entrykey->k, val = (struct json_object*)entrykey->v, entry_nextkey = entrykey->next, entrykey) : 0);
		 entrykey = entry_nextkey) {

		if (key == NULL) {
			printf("[-] Error :: json_parse_obj_rec :: uninitialized key char*\n");
			continue;
		}
		
		// ignore header.
		if (strncmp(key, "content", 6) != 0 && strncmp(key, "packageList", 6) != 0)
			continue;

		//printf("[+] Debug :: json_parse_obj_rec :: key = %s\n", key);

		type = json_object_get_type(val);

		switch (type) {

			case json_type_boolean:
				break;
			case json_type_double:
				break;
			case json_type_int:
				break;
			case json_type_string:
				break;
			case json_type_object:
								
				array_len = json_parse_obj_rec(val,pkgList);
				//printf("[+] Debug :: json_parse_obj_rec :: (filter) key = %s :: array_len = %d\n",key, array_len);				
				break;
				
			case json_type_array:

				//printf("[+] Debug :: json_parse_obj_rec :: key = %s\n", key);

				// allocate memory for package list.
				if ((array_len = json_object_array_length(val) ) <= 0) {
					printf("[-] Error :: json_parse_obj_rec :: Empty array!\n");
					break;
				}

				// do it only if the pkg list is already allocated (second call).
				if ( (pkgList != NULL) && ( (array_len = json_parse_array(val, pkgList)) <= 0) ) {
					printf("[-] Error :: json_parse_obj_rec :: json parse array failed! :: array_len = %d :: pkglist =%d\n",array_len,pkgList);
					break;
				}							
				break;
			default:
				printf(" [+] Error :: json_parse_obj_rec :: Unknown json_object type :: %d \n",type);
				break;

		}

	}

	return array_len;

}