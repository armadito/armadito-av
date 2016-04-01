#ifndef _LIBARMADITO_ONDEMAND_H_
#define _LIBARMADITO_ONDEMAND_H_

#include <libarmadito/scan.h>

struct a6o_on_demand;

struct a6o_on_demand *a6o_on_demand_new(struct armadito *armadito, int scan_id, const char *root_path, enum a6o_scan_flags flags);

struct a6o_scan *a6o_on_demand_get_scan(struct a6o_on_demand *on_demand);

void a6o_on_demand_run(struct a6o_on_demand *on_demand);

void a6o_on_demand_free(struct a6o_on_demand *on_demand);

#endif
