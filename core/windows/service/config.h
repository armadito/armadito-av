#ifndef __UH_CONF_H__
#define __UH_CONF_H__

#include <libarmadito.h>
#include <Windows.h>

struct conf_reg_data{
	HKEY hRootKey;
	HKEY hSectionKey;
	char * path;
	char * prev_section;
};

void display_entry(const char *section, const char *key, struct a6o_conf_value *value, void *user_data);
int save_conf_in_registry(struct a6o_conf * conf);
int restore_conf_from_registry(struct a6o_conf * conf);

int conf_poc_windows( );
int disable_onaccess( );

#endif