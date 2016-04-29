#ifndef _LIBARMADITO_SCANP_H_
#define _LIBARMADITO_SCANP_H_

#include <glib.h>

struct a6o_scan {
	GArray *callbacks;                  /* array of struct callback_entry */

	int scan_id;                        /* scan id for GUI */

	int to_scan_count;                  /* files to scan counter, to compute progress */
	int scanned_count;                  /* already scanned counter, to compute progress */
        int malware_count;                /* detected as malicious counter */
	int suspicious_count;               /* detected as suspicious counter */
};

void a6o_scan_call_callbacks(struct a6o_scan *scan, struct a6o_report *report);

#endif
