#include "scandialog.h"
#include "ui/glade_scandialog.h"
#include "model/scanmodel.h"

#include <glib/gi18n.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void close_cb(GtkWidget *widget, gpointer user_data)
{
  struct scan_dialog *sd = (struct scan_dialog *)user_data;

  gtk_widget_destroy(GTK_WIDGET(sd->dialog_widget));
}

static void count_changed_cb(GObject *obj, guint counter_type, guint counter_value, gpointer user_data)
{
  struct scan_dialog *sd = (struct scan_dialog *)user_data;
  GtkEntry *entry;
  char *tmp;

  fprintf(stderr, "count changed %s %d\n", scan_counter_type_str(counter_type), counter_value);

  switch(counter_type) {
  case TO_SCAN_COUNTER:
    entry = sd->toScanCount;
    sd->to_scan_count = counter_value;
    break;
  case SCANNED_COUNTER:
    entry = sd->scannedCount;
    if (sd->to_scan_count != 0) {
      gtk_progress_bar_set_fraction(sd->scanProgress, counter_value / (double)sd->to_scan_count);
    }
    break;
  case MALWARE_COUNTER:
    entry = sd->malwareCount;
    break;
  case SUSPICIOUS_COUNTER:
    entry = sd->suspiciousCount;
    break;
  case UNHANDLED_COUNTER:
    entry = sd->unhandledCount;
    break;
  case CLEAN_COUNTER:
    entry = sd->cleanCount;
    break;
  }

  tmp = (char *)malloc(64);
  sprintf(tmp, "%d", counter_value);

  gtk_entry_set_text(entry, tmp);

  free(tmp);
}

static const char *file_status_pretty_str_i18n(enum uhuru_file_status status)
{
  switch(status) {
  case UHURU_UNDECIDED:
    return _("status undecided");
  case UHURU_CLEAN:
    return _("status clean");
  case UHURU_UNKNOWN_FILE_TYPE:
    return _("status ignored");
  case UHURU_EINVAL:
    return _("status invalid argument");
  case UHURU_IERROR:
    return _("status internal error");
  case UHURU_SUSPICIOUS:
    return _("status suspicious");
  case UHURU_WHITE_LISTED:
    return _("status white listed");
  case UHURU_MALWARE:
    return _("status malware");
  }

  return _("status unknown");
}

static const char *action_pretty_str_i18n(enum uhuru_action action)
{
  switch(action & (UHURU_ACTION_ALERT | UHURU_ACTION_QUARANTINE | UHURU_ACTION_REMOVE)) {
  case UHURU_ACTION_ALERT: return _("action alert");
  case UHURU_ACTION_ALERT | UHURU_ACTION_QUARANTINE: return _("action alert+quarantine");
  case UHURU_ACTION_ALERT | UHURU_ACTION_REMOVE: return _("action alert+removed");
  }

  return _("alert none");
}

static void scanning_cb(GObject *obj, guint u_status, guint u_action, const gchar *path, gdouble scan_duration, gpointer user_data)
{
  struct scan_dialog *sd = (struct scan_dialog *)user_data;
  enum uhuru_file_status status = (enum uhuru_file_status)u_status;
  enum uhuru_action action = (enum uhuru_action)u_action;
  char *tmp;

  gtk_progress_bar_set_text(sd->scanProgress, path);

  tmp = (char *)malloc(64);
  sprintf(tmp, "%.2f", scan_duration);
  gtk_entry_set_text(sd->scan_duration, tmp);
  free(tmp);

  if (status != UHURU_WHITE_LISTED && status != UHURU_CLEAN) {
    GtkTreeIter iter;

    gtk_tree_store_append(sd->reportStore, &iter, NULL);
    gtk_tree_store_set(sd->reportStore, &iter,
		       0, file_status_pretty_str_i18n(status),
		       1, action_pretty_str_i18n(action),
		       2, path,
		       -1);
  }
}

static void completed_cb(GObject *obj, gpointer user_data)
{
  struct scan_dialog *sd = (struct scan_dialog *)user_data;

  gtk_widget_set_sensitive(GTK_WIDGET(sd->closeButton), TRUE);
}

static void scan_dialog_init_from_builder(struct scan_dialog *sd)
{
  GtkBuilder *builder; 
  GError *err = NULL;

  builder = gtk_builder_new();
  if(gtk_builder_add_from_string(builder, scandialog_builder_def, strlen(scandialog_builder_def), &err) == 0) {
    fprintf(stderr, "Error adding build from string. Error: %s\n", err->message);

    exit(1);
  }

  sd->dialog_widget = GTK_DIALOG(gtk_builder_get_object(builder, "scanDialog"));
  gtk_builder_connect_signals(builder, NULL);
  
  sd->path = GTK_ENTRY(gtk_builder_get_object(builder, "path"));

  sd->toScanCount = GTK_ENTRY(gtk_builder_get_object(builder, "toScanCount"));
  sd->scannedCount = GTK_ENTRY(gtk_builder_get_object(builder, "scannedCount"));
  sd->malwareCount = GTK_ENTRY(gtk_builder_get_object(builder, "malwareCount"));
  sd->suspiciousCount = GTK_ENTRY(gtk_builder_get_object(builder, "suspiciousCount"));
  sd->unhandledCount = GTK_ENTRY(gtk_builder_get_object(builder, "unhandledCount"));
  sd->cleanCount = GTK_ENTRY(gtk_builder_get_object(builder, "cleanCount"));

  sd->scan_duration = GTK_ENTRY(gtk_builder_get_object(builder, "scanDuration"));
  
  sd->scanProgress = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "scanProgress"));

  sd->cancelButton = GTK_BUTTON(gtk_builder_get_object(builder, "cancelButton"));
  sd->closeButton = GTK_BUTTON(gtk_builder_get_object(builder, "closeButton"));

  sd->reportStore = GTK_TREE_STORE(gtk_builder_get_object(builder, "reportStore"));

  g_object_unref(G_OBJECT(builder));
}

struct scan_dialog *scan_dialog_new(GObject *model)
{
  struct scan_dialog *sd;
  gchar *path_val;

  sd = (struct scan_dialog *)malloc(sizeof(struct scan_dialog));

  scan_dialog_init_from_builder(sd);

  g_signal_connect(sd->closeButton, "clicked", G_CALLBACK(close_cb), sd);

  sd->to_scan_count = 0;

  g_signal_connect(model, "count_changed", (GCallback)count_changed_cb, sd);
  g_signal_connect(model, "scanning", (GCallback)scanning_cb, sd);
  g_signal_connect(model, "completed", (GCallback)completed_cb, sd);

  g_object_get(model, "scan-path", &path_val, NULL);
  gtk_entry_set_text(sd->path, path_val);
  g_free(path_val);

  /* scan_model_reemit(SCAN_MODEL(model)); */

  return sd;
}

#if 0
void scan_dialog_set_model(struct scan_dialog *sd, GObject *model)
{
  g_signal_connect(model, "count_changed", (GCallback)count_changed_cb, sd);
  g_signal_connect(model, "scanning", (GCallback)scanning_cb, sd);
  g_signal_connect(model, "completed", (GCallback)completed_cb, sd);

  /* gtk_entry_set_text(sd->path, path); */
}
#endif
