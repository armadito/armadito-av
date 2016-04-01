#ifndef _LIBARMADITO_ARMADITOP_H_
#define _LIBARMADITO_ARMADITOP_H_

#include <libarmadito.h>

struct a6o_module **a6o_get_modules(struct armadito *u);

struct a6o_module *a6o_get_module_by_name(struct armadito *u, const char *name);

#ifdef DEBUG
const char *a6o_debug(struct armadito *u);
#endif

#endif
