#ifndef _LIBARMADITO_INFO_H_
#define _LIBARMADITO_INFO_H_

#include <libarmadito/status.h>
#include <libarmadito/handle.h>

enum a6o_update_status {
	ARMADITO_UPDATE_OK,
	ARMADITO_UPDATE_LATE,
	ARMADITO_UPDATE_CRITICAL,
	ARMADITO_UPDATE_NON_AVAILABLE,
};

const char *a6o_update_status_str(enum a6o_update_status status);

struct a6o_base_info {
	const char *name;
	/* UTC and ISO 8601 date */
	const char *date;
	const char *version;
	unsigned int signature_count;
	const char *full_path;
};

struct a6o_module_info {
	const char *name;
	enum a6o_update_status mod_status;
	/* UTC and ISO 8601 date time */
	const char *update_date;
	/* NULL terminated array of pointers to struct base_info */
	struct a6o_base_info **base_infos;
};

struct a6o_info {
	enum a6o_update_status global_status;
	/* NULL terminated array of pointers to struct a6o_module_info */
	struct a6o_module_info **module_infos;
};

struct a6o_info *a6o_info_new(struct armadito *armadito);

void a6o_info_to_stdout(struct a6o_info *info);

void a6o_info_free(struct a6o_info *info);

#endif
