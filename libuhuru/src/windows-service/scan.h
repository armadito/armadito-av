#ifndef _TEST_SCAN_
#define _TEST_SCAN_

#include <libuhuru-config.h>
#include <libuhuru/core.h>

struct new_scan{
	int scan_id;
	const char *scan_path;
};

int start_new_scan(struct new_scan* scan, uhuru* uhuru);

#endif