#ifndef _LIBUHURU_SCANP_H_
#define _LIBUHURU_SCANP_H_

#include <glib.h>

struct uhuru_scan {
  GArray *callbacks;                  /* array of struct callback_entry */

  int scan_id;                        /* scan id for GUI */

  int to_scan_count;                  /* files to scan counter, to compute progress */
  int scanned_count;                  /* already scanned counter, to compute progress */
};

void uhuru_scan_call_callbacks(struct uhuru_scan *scan, struct uhuru_report *report);

#endif
