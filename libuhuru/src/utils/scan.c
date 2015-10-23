#include <stdio.h>
#include "scan.h"

int start_new_scan(struct new_scan* scan)
{
	if (scan->scan_id > 0 && scan->scan_path != NULL){
		printf("\n\n #### Start scanning (%d) %s ####\n", scan->scan_id, scan->scan_path);
    } 

	return 0;
}
