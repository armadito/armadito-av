#ifndef _UI_SCANDIALOG_H_
#define _UI_SCANDIALOG_H_

#include <glib.h>
#include <gtk/gtk.h>
#include <libumwsu/scan.h>

struct scan_dialog {
  GtkDialog *dialog_widget;
  GtkEntry *path;
  GtkEntry *toScanCount, *scannedCount, *malwareCount, *suspiciousCount, *unhandledCount, *cleanCount;
  GtkEntry *scan_duration;
  GtkProgressBar *scanProgress;
  GtkButton *cancelButton, *closeButton;
  GtkTreeStore *reportStore;
  int to_scan_count;
};

struct scan_dialog *scan_dialog_new(GObject *model);

#endif
