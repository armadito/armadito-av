#ifndef _LIBARMADITO_ON_ACCESS_H_
#define _LIBARMADITO_ON_ACCESS_H_

#include <libarmadito.h>


struct onaccess_data {
	int enable_permission;
	int state_flag;
};

extern struct a6o_module on_access_win_module;

#endif
