#include "config.h"
#include <ui\ui.h>

#define APP_ROOT_KEY_PATH "SOFTWARE\\Teclib\\Armadito"
#define COMPAGNY_NAME "Teclib"
#define PRODUCT_NAME "Armadito-av"

#define SVC_IPC_PATH "\\\\.\\pipe\\Armadito_ondemand"

#define MAX_LEN 512

int create_app_registry( ) {

	int ret = 0;
	HKEY hRootKey = NULL;
	HKEY hAppKey = NULL;
	DWORD dwDispo = 0;


	__try {

		if (RegOpenKeyA(HKEY_LOCAL_MACHINE,NULL, &hRootKey) != ERROR_SUCCESS) {
			printf("[-] Error :: RegOpenKeyA failed! :: GLE= %d\n", GetLastError( ));
			__leave;
		}


		if(RegCreateKeyA(hRootKey,APP_ROOT_KEY_PATH,&hAppKey) != ERROR_SUCCESS) {
			printf("[-] Error :: RegCreateKeyA failed! :: GLE= %d\n", GetLastError( ));
			// TODO :: if the key is already created.
			__leave;
		}

		/*if (hAppKey != NULL) {
			RegCloseKey(hAppKey);
			hAppKey = NULL;
		}*/


		/*if (RegCreateKeyA(hRootKey,APP_ROOT_KEY_PATH,&hAppKey) != ERROR_SUCCESS) {
			printf("[-] Error :: RegCreateKeyA failed! :: GLE= %d\n", GetLastError( ));
			// TODO :: if the key is already created.
			__leave;
		}*/

		/*if (RegCreateKeyExA(hRootKey,APP_ROOT_KEY_PATH,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hAppKey,&dwDispo) != ERROR_SUCCESS) {
			printf("[-] Error :: RegCreateKeyExA failed! :: GLE= %d\n", GetLastError( ));
			// TODO :: if the key is already created.
			__leave;
		}*/

		


		printf("[+] Debug :: Application Registry key created successfully!\n");

		/*if (RegDeleteKeyA(hAppKey,NULL) != ERROR_SUCCESS) {
			printf("[-] Error :: RegDeleteKeyA failed! :: GLE= %d\n", GetLastError( ));
			// TODO :: if the key is already created.
			__leave;
		}*/

		//printf("[+] Debug :: Application Registry key deleted successfully!\n");



	}
	__finally {

		if (hAppKey != NULL) {
			RegCloseKey(hAppKey);
			hAppKey = NULL;
		}

		if (hRootKey != NULL) {
			RegCloseKey(hRootKey);
			hRootKey = NULL;
		}



	}



	return ret;
}

int delete_app_registry( ) {

	int ret = 0;
	HKEY hRootKey = NULL;

	__try {

		if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE", &hRootKey) != ERROR_SUCCESS) {
			printf("[-] Error :: RegOpenKeyA failed with error :: %d\n", GetLastError( ));
			__leave;
		}

	}
	__finally {

		if (hRootKey != NULL) {
			RegCloseKey(hRootKey);
			hRootKey = NULL;
		}

	}

	return ret;
}

void write_entry_in_registry(const char *section, const char *key, struct a6o_conf_value *value, void *user_data){
	
	struct conf_reg_data * data = (struct conf_reg_data *)user_data;
	int n = 0, i = 0;
	int vlen = 0;
	char * val = NULL, *tmp = NULL;
	char ** list_val = NULL;
	HKEY hSectionKey = NULL;
	HKEY hRootKey = NULL;
	int reg_val_type = 0;
	int reg_val_len = 0;
	void* reg_val_data = NULL;
	
	int res = 0;


	if (data->prev_section == NULL || strcmp(data->prev_section, section)) {

		printf("\n[%s]\n\n", section);

		// create section registry key.
		if (data->hRootKey != NULL) {

			
			if((res = RegCreateKeyA(data->hRootKey, section, &hSectionKey)) != ERROR_SUCCESS) {				
				printf("[-] Error :: RegCreateKeyA failed! :: GLE= %d\n", res);
				return;
			}

			if (data->hSectionKey != NULL) {
				RegCloseKey(data->hSectionKey);
				data->hSectionKey = NULL;
			}

			data->hSectionKey = hSectionKey;			
		}
		
		data->prev_section = section;
	}


	printf("\tkey = %s :: type= %d :: ", key,value->type);
	switch (value->type) {

		case CONF_TYPE_VOID:
			printf("[-] Warning :: Invalid conf type\n");
			return;
			break;

		case CONF_TYPE_INT:
			reg_val_type = REG_DWORD;
			reg_val_len = sizeof(DWORD);
			reg_val_data = &value->v;
			printf("value = %d",value->v);
			break;

		case CONF_TYPE_STRING:
			reg_val_type = REG_SZ;
			reg_val_data = a6o_conf_value_get_string(value);
			reg_val_len = strnlen_s(reg_val_data,MAX_LEN) +1;			
			printf("value = %s :: ",reg_val_data);
			printf("value_len = %d",reg_val_len);
			break;

		case CONF_TYPE_LIST:
			reg_val_type = REG_MULTI_SZ;
			
			
			list_val = a6o_conf_value_get_list(value);
			for (i = 0; i < a6o_conf_value_get_list_len(value); i++) {
				printf("value = %s :: ",*list_val);				
				reg_val_len += (strnlen_s(*list_val,MAX_LEN) +1);
				list_val++;				
			}

			
			printf("value_len = %d :: ",reg_val_len);
			val = (char*)calloc(reg_val_len + 1, sizeof(char));
			val[reg_val_len] = '\0';
			reg_val_len++; // '\0' (two null characters at the end).
			tmp = val;

			list_val = a6o_conf_value_get_list(value);
			for (i = 0; i < a6o_conf_value_get_list_len(value); i++) {

				memcpy(tmp,*list_val,(strnlen_s(*list_val,MAX_LEN) +1));
				tmp += (strnlen_s(*list_val, MAX_LEN) + 1);
				list_val++;

			}

			reg_val_data = val;

			break;
			
		default:			
			printf("[-] Warning :: Invalid conf type :: [%d]\n",value->type);
			return;
			break;
	}


	res = RegSetKeyValueA(data->hSectionKey, NULL, key, reg_val_type, reg_val_data, reg_val_len);
	if (res != ERROR_SUCCESS) {
		printf("[-] Error :: RegSetKeyValueA failed! :: GLE = %d\n", res);
		if (val != NULL) {
			free(val);
			val = NULL;
		}
	
		return;
	}

	if (val != NULL) {
		free(val);
		val = NULL;
	}


	printf("\n");

	return;
}

void display_entry(const char *section, const char *key, struct a6o_conf_value *value, void *user_data){
	
	struct conf_reg_data * data = (struct conf_reg_data *)user_data;
	int n = 0, i = 0;
	int vlen = 0;
	char * val = NULL, *tmp = NULL;
	char ** list_val = NULL;
	HKEY hSectionKey = NULL;
	HKEY hRootKey = NULL;
	int reg_val_type = 0;
	int reg_val_len = 0;
	void* reg_val_data = NULL;
	
	int res = 0;


	if (data->prev_section == NULL || strcmp(data->prev_section, section)) {

		printf("\n[%s]\n\n", section);		
		data->prev_section = section;
	}


	printf("\tkey = %s :: type= %d :: ", key,value->type);
	switch (value->type) {

		case CONF_TYPE_VOID:
			printf("[-] Warning :: Invalid conf type\n");
			return;
			break;

		case CONF_TYPE_INT:
			reg_val_type = REG_DWORD;
			reg_val_len = sizeof(DWORD);
			reg_val_data = &value->v;
			printf("value = %d",value->v);
			break;

		case CONF_TYPE_STRING:
			reg_val_type = REG_SZ;
			reg_val_data = a6o_conf_value_get_string(value);
			reg_val_len = strnlen_s(reg_val_data,MAX_LEN) +1;			
			printf("value = %s :: ",reg_val_data);
			printf("value_len = %d",reg_val_len);
			break;

		case CONF_TYPE_LIST:
			reg_val_type = REG_MULTI_SZ;
			
			
			list_val = a6o_conf_value_get_list(value);
			for (i = 0; i < a6o_conf_value_get_list_len(value); i++) {
				printf("value = %s :: ",*list_val);				
				reg_val_len += (strnlen_s(*list_val,MAX_LEN) +1);
				list_val++;				
			}

			
			printf("value_len = %d :: ",reg_val_len);
			val = (char*)calloc(reg_val_len + 1, sizeof(char));
			val[reg_val_len] = '\0';
			reg_val_len++; // '\0' (two null characters at the end).
			tmp = val;

			list_val = a6o_conf_value_get_list(value);
			for (i = 0; i < a6o_conf_value_get_list_len(value); i++) {

				memcpy(tmp,*list_val,(strnlen_s(*list_val,MAX_LEN) +1));
				tmp += (strnlen_s(*list_val, MAX_LEN) + 1);
				list_val++;

			}

			reg_val_data = val;

			break;
			
		default:			
			printf("[-] Warning :: Invalid conf type :: [%d]\n",value->type);
			return;
			break;
	}

	if (val != NULL) {
		free(val);
		val = NULL;
	}


	printf("\n");

	return;
}

int save_conf_in_registry(struct a6o_conf * conf) {

	int ret = 0;
	HKEY hRootKey = NULL;
	struct conf_reg_data data = {0};
	int res = 0;

	if (conf == NULL) {
		printf("[-] Error :: save_conf_in_registry :: invalid parameter\n");
		return -1; 
	}	

	__try {

		// Open Antivirus root key.
		if (RegOpenKeyA(HKEY_LOCAL_MACHINE,APP_ROOT_KEY_PATH, &hRootKey) != ERROR_SUCCESS) {
			printf("[-] Error :: RegOpenKeyA failed! :: GLE= %d\n", GetLastError( ));
			ret = -2;
			__leave;
		}

		//
		printf("[+] Debug :: Antivirus root key opened successfuly!\n");
		printf("hRootKey = %lu\n",hRootKey);
		data.hRootKey = hRootKey;
		data.hSectionKey = NULL;
		data.path = NULL;
		data.prev_section = NULL;

		// go through all sections
		a6o_conf_apply(conf,(a6o_conf_fun_t)write_entry_in_registry, &data);

	}
	__finally {

		if (hRootKey != NULL) {
			RegCloseKey(hRootKey);
			hRootKey = NULL;
		}

		if (data.hSectionKey != NULL) {
			RegCloseKey(data.hSectionKey);
			data.hSectionKey = NULL;
		}

	}

	

	return ret;
}

int registry_walker(HKEY hkey, const char * section,  struct a6o_conf * conf){

	int ret = 0;
	HKEY hSubKey = NULL;
	char * subkey_name = NULL;
	int subkey_maxlen = 0;
	int subkey_len = 0;
	char * class = NULL;
	char * value_name = NULL;
	int value_len = 0;
	int value_maxlen = 0;
	int value_type =0;
	int data_len = 0;
	char * data_str = NULL;
	int data_int = 0;
	char ** data_list = NULL;
	char * tmp = NULL;
	int list_len = 0;
	int class_len = 0;
	int class_maxlen = 0;
	int res = 0;
	int nb_index = 0, i =0,j=0;
	int nbvalues = 0;
	struct conf_reg_data data = {0};
	

	if (hkey == NULL) {
		printf("[-] Error :: registry_walker :: invalid parameter!\n");
		return -1;
	}

	__try {


		res = RegQueryInfoKeyA(hkey, NULL, &class_len, NULL, &nb_index, &subkey_maxlen, &class_maxlen, &nbvalues, &value_maxlen, NULL,NULL,NULL);
		if (res != ERROR_SUCCESS) {
			printf("[-] Error :: RegEnumKeyExA failed! :: GLE= %d\n", res);
			ret = -3;
			__leave;
		}
		//printf("[+] Debug :: nbindex = %d :: nbvalues = %d\n",nb_index,nbvalues);
		//printf("[+] Debug :: subkey_maxlen = %d :: index = %d :: class_maxlen = %d :: nbvalues = %d\n",subkey_maxlen ,nb_index, class_maxlen, nbvalues);


		value_maxlen++;
		value_name = (char *)calloc(value_maxlen + 1, sizeof(char));

		// Get values.
		for (i = 0; i < nbvalues; i++) {

			value_len = value_maxlen;
			if ((res = RegEnumValueA(hkey, i, value_name, &value_len, NULL, &value_type, NULL, &data_len)) != ERROR_SUCCESS) {
				printf("[-] Error :: RegEnumValueA failed! :: GLE= %d\n", res);
				ret = -3;
				__leave;
			}

			// get data.
			switch (value_type) {

				case REG_DWORD:					
					if ((res = RegGetValueA(hkey,NULL,value_name,RRF_RT_ANY, NULL, &data_int,&data_len)) != ERROR_SUCCESS ) {
						printf("[-] Error :: RegEnumValueA failed! :: GLE= %d\n", res);
						break;
					}
					//printf("[+] Debug :: value_name = %s :: value_type = %d :: data_len = %d :: data_int = %d\n",value_name,value_type,data_len,data_int );
					//printf("[+] Debug :: (%s) => %d\n",value_name,data_int );

					// add value to conf struct
					if (section != NULL)
						a6o_conf_add_uint(conf, section, value_name, data_int);

					break;
				case REG_SZ:

					//printf("[+] Debug :: value_name = %s :: value_type = %d :: data_len = %d\n",value_name,value_type,data_len );					
					data_str = (char *)calloc(data_len + 1, sizeof(char));
					if ((res = RegGetValueA(hkey,NULL,value_name,RRF_RT_ANY, NULL,data_str,&data_len)) != ERROR_SUCCESS ) {
						printf("[-] Error :: RegEnumValueA failed! :: GLE= %d\n", res);
						break;
					}
					//printf("[+] Debug :: (%s) => %s\n",value_name,data_str);

					if (section != NULL)
						a6o_conf_add_string(conf, section, value_name, data_str);
					break;


				case REG_MULTI_SZ:

					//data_string
					//printf("[+] Debug :: value_name = %s :: value_type = %d :: data_len = %d\n",value_name,value_type,data_len );

					data_str = (char *)calloc(data_len + 1, sizeof(char));
					if ((res = RegGetValueA(hkey,NULL,value_name,RRF_RT_ANY, NULL,data_str,&data_len)) != ERROR_SUCCESS ) {
						printf("[-] Error :: RegEnumValueA failed! :: GLE= %d\n", res);
						break;
					}

					//printf("[+] Debug :: [%s] => %s\n",value_name,data_str);
					//printf("[+] Debug :: (%s) =>\n",value_name);

					list_len = 0;
					tmp = data_str;
					for (; *tmp != '\0'; tmp += strnlen(tmp,MAX_PATH)+1) {
						//printf("\t\tvalue = %s\n",tmp);
						list_len++;
					}
					///printf("\n");

					data_list = (char**)calloc(list_len, sizeof(char*));
					tmp = data_str;
					j = 0;
					//printf("list_len = %d\n",list_len);
					for (; *tmp != '\0'; tmp += strnlen(tmp, MAX_PATH) + 1) {
						
						data_list[j] = (char*)calloc(strnlen(tmp, MAX_PATH) + 1,sizeof(char));
						memcpy(data_list[j],tmp,strnlen(tmp, MAX_PATH) + 1);						
						//printf("\tvalue = %s\n",data_list[j]);
						
						j++;
					}
					
					if (section != NULL)
						a6o_conf_add_list(conf, section, value_name, data_list, list_len);


					if (data_list != NULL) {
						for (j = 0; j < list_len; j++) {
							free(data_list[j]);
							data_list[j] = NULL;
						}
						free(data_list);
						data_list = NULL;
					}

					if (data_str != NULL) {
						free(data_str);
						data_str = NULL;
					}

					break;

				default:
					//printf("[+] Debug :: value_name = %s :: value_type = %d :: data_len = %d\n",value_name,value_type,data_len );
					break;
				
			}

		}


		// Get subkeys
		subkey_maxlen++; // add null terminating character.
		subkey_name = (char *)calloc(subkey_maxlen + 1, sizeof(char));

		for (i = 0; i < nb_index; i++) {

			subkey_len = subkey_maxlen;
			res = RegEnumKeyExA(hkey, i, subkey_name, &subkey_len, NULL, NULL, NULL, NULL);
			if (res != ERROR_SUCCESS) {
				printf("[-] Error :: RegEnumKeyExA failed! :: GLE= %d\n", res);
				ret = -3;
				__leave;
			}

			printf("[+] Debug :: section = [%s]\n",subkey_name);
			//printf("[+] Debug :: sub_key_name = [%s] :: sub_key_len = %d :: class_len = %d\n",subkey_name, subkey_len,class_len);

			
			if ((res = RegOpenKeyA(hkey,subkey_name, &hSubKey)) != ERROR_SUCCESS) {
				printf("[-] Error :: RegOpenKeyA failed! :: GLE= %d\n", res);
				ret = -2;
				__leave;
			}


			registry_walker(hSubKey,subkey_name,conf);

			//printf("\n\n");

		}


		


		



	}
	__finally {

		if (subkey_name != NULL) {
			free(subkey_name);
			subkey_name = NULL;
		}

		if (value_name != NULL) {
			free(value_name);
			value_name = NULL;
		}

		if (hSubKey != NULL) {
			RegCloseKey(hSubKey);
			hSubKey = NULL;
		}

		if (data_str != NULL) {
			free(data_str);
			data_str = NULL;
		}

		if (data_list != NULL) {
			for (j = 0; j < list_len; j++) {
				free(data_list[j]);
				data_list[j] = NULL;
			}
			free(data_list);
			data_list = NULL;
		}


		

		

	}

	




	return ret;
}

int restore_conf_from_registry(struct a6o_conf * conf) {

	int ret = 0;	
	HKEY hAppKey = NULL;
	int res = 0;
	struct conf_reg_data data = {0};


	if (conf == NULL) {
		printf("[-] Error :: invalid parameter\n");
		return -1;
	}

	__try {

		if ((res = RegOpenKeyA(HKEY_LOCAL_MACHINE,APP_ROOT_KEY_PATH, &hAppKey)) != ERROR_SUCCESS) {
			printf("[-] Error :: RegOpenKeyA failed! :: GLE= %d\n", res);
			ret = -2;
			__leave;
		}

		registry_walker(hAppKey,NULL,conf);


		//printf("\n\n UHURU_CONF :::\n\n");		
		// display a6o_conf
		//a6o_conf_apply(conf,(a6o_conf_fun_t)display_entry, &data);
		

	}
	__finally {

		if (hAppKey != NULL) {
			RegCloseKey(hAppKey);
			hAppKey = NULL;
		}

	}


	return ret;
}

int conf_poc_windows(char * conf_file_path ) {
	
	struct a6o_conf * conf = NULL;
	a6o_error * error = NULL;
	int ret = 0;


	ret = create_app_registry( );

	// Initialize config structure.
	conf = a6o_conf_new();
	if (conf == NULL) {
		printf("[-] Error :: conf struct initialization failed!\n");
	}

	printf("[+] Debug :: conf initialized successfully!\n");

	// Load config from config file. a6o.conf
	a6o_conf_load_file(conf, conf_file_path, &error);

	// save conf structure in regsitry.
	save_conf_in_registry(conf);

	printf("\n");
	printf("[+] Debug :: conf saved to registry successfully!\n");

	printf("\n\n");

	//
	a6o_conf_free(conf);
	conf = NULL;

	// restore conf from registry
	conf = a6o_conf_new();
	restore_conf_from_registry(conf);


	a6o_conf_free(conf);
	conf = NULL;

	return ret;

}

int disable_onaccess() {

	int ret = 0;
	json_object * jobj = NULL;
	json_object * jparams = NULL;
	char * request = NULL;
	int req_len = 0;
	char response[2048];
	int response_len = 2048;
	enum a6o_json_status status = JSON_OK;


	__try {

		if ((jobj = json_object_new_object()) == NULL) {
			printf("[-] Error :: disable_onaccess :: can't create json object!\n");
			ret = -2;
			__leave;
		}

		if ((jparams = json_object_new_object()) == NULL) {
			printf("[-] Error :: disable_onaccess :: can't create json object!\n");
			ret = -3;
			__leave;
		}		

		/*
		{ 
			"av_request":"conf_set",
			"id":123,			
			"params": {
				"section":"on-access"
				"key":"enable",
				"value":1
			} 
		}
		*/

		json_object_object_add(jobj, "av_request", json_object_new_string("conf_set"));
		json_object_object_add(jobj, "id", json_object_new_int(123));

		json_object_object_add(jparams,"section", json_object_new_string("on-access"));
		json_object_object_add(jparams, "key", json_object_new_string("enable"));
		json_object_object_add(jparams, "value", json_object_new_int(0));
		json_object_object_add(jobj, "params", jparams);
			
		request = json_object_get_string(jobj);
		req_len = strnlen(request,MAX_LEN);
		
		status = json_handler_ui_request(SVC_IPC_PATH, request, req_len, response, response_len);
		if (status != JSON_OK) {
			//printf("[-] Warning :: send_notif :: notification not sent/received correctly!\n");
			a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_WARNING,"[-] Warning :: disable_onaccess :: notification not sent/received correctly!\n");
			ret = -4;
			__leave;
		}

		printf("[+] Debug :: disable_onaccess :: notification = %s\n",request);



	}
	__finally {

		if (jobj != NULL) {
			json_object_put(jobj);
		}

		if (jparams != NULL) {
			json_object_put(jparams);
		}
	}


	return ret;

}

/*This function set the registry from the configuration file.*/
int init_configuration( ) {

	int ret = 0;
	char * conf_file = NULL;
	struct a6o_conf * conf = NULL;
	a6o_error * error = NULL;

	__try {		

		// Create registry keys.
		if (create_app_registry( ) != 0) {
			a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: init_configuration :: registry key creation failed!\n");
			ret = -1;
			__leave;
		}

		printf("[+] Debug :: init_configuration :: registry keys created successfully!\n");

		// Load configuration from file.	
		if ((conf = a6o_conf_new()) == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: init_configuration :: conf struct initialization failed!\n");
			ret = -2;
			__leave;
		}

		// get configuration file path.
		conf_file = a6o_std_path(CONFIG_FILE_LOCATION);
		if (conf_file == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: init_configuration :: get configuration file path failed !\n");
			ret = -3;
			__leave;
		}

		printf("[+] Debug :: init_configuration :: conf file = %s\n",conf_file);

		// Load config from config file. a6o.conf
		a6o_conf_load_file(conf, conf_file, &error);

		printf("[+] Debug :: init_configuration :: configuration file loaded successfully!\n");


		// save conf structure in registry.
		save_conf_in_registry(conf);

		printf("[+] Debug :: init_configuration :: configuration file saved successfully!\n");



	}
	__finally {

		if (conf_file != NULL) {
			free(conf_file);
			conf_file = NULL;
		}

		if (conf != NULL) {
			a6o_conf_free(conf);
			conf = NULL;
		}

	}

	return ret;

}