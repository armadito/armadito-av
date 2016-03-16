#ifndef __UH_JSON_PARSE_H__
#define __UH_JSON_PARSE_H__
#endif

#include <stdio.h>
#include <string.h>
#include <json.h>


typedef struct packageStruct{
	char * displayname;
	char * fileurl;
	char * controlsum;
	char * controltype;
	char * licence;
	char * cachefilename;
}Package ;


int fillPackageParam(Package** pkgList, int index, char * key, char * value);
int json_parse_obj_and_process(json_object * obj, int index, Package** pkgList);
int json_parse_array(json_object * jarray, Package ** pkgList);
int json_parse_obj_rec(json_object * obj, Package ** pkgList);