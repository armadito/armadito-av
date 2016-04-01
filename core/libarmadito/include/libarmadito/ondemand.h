#ifndef _LIBUHURU_LIBCORE_ONDEMAND_H_
#define _LIBUHURU_LIBCORE_ONDEMAND_H_

#include <libuhuru/libcore/scan.h>

#ifdef __cplusplus
extern "C" {
#endif

struct uhuru_on_demand;

struct uhuru_on_demand *uhuru_on_demand_new(struct uhuru *uhuru, int scan_id, const char *root_path, enum uhuru_scan_flags flags);

struct uhuru_scan *uhuru_on_demand_get_scan(struct uhuru_on_demand *on_demand);

void uhuru_on_demand_run(struct uhuru_on_demand *on_demand);

void uhuru_on_demand_free(struct uhuru_on_demand *on_demand);

#ifdef __cplusplus
}
#endif

#endif
