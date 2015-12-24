#ifndef _TEST_SCAN_
#define _TEST_SCAN_

#include <libuhuru-config.h>
#include <libuhuru/core.h>
#include <windows.h>




struct new_scan_action{
	int scan_id;
	const char *scan_path;
	const char *scan_action;
};

int start_new_scan(struct new_scan_action* scan, struct uhuru* uhuru);
int cancel_current_scan(struct new_scan_action* scan, struct uhuru* uhuru);

#endif