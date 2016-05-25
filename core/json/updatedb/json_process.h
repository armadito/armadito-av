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

#ifndef __ARMADITO_JSON_PARSE_H__
#define __ARMADITO_JSON_PARSE_H__
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
