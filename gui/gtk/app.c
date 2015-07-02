#include "uhuru-linux-gui-config.h"
#include "ui/systraymenu.h"
#include "ui/scandialog.h"
#include "model/scanmodel.h"
#include "app.h"
#include "utils/dbus.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libnotify/notify.h>
#ifdef HAVE_APPINDICATOR
#include <libappindicator/app-indicator.h>
#endif
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

struct scan_entry {
  GObject *model;
  GtkWidget *item;
  struct scan_dialog *dialog;
};

struct uhuru_app {
  GPtrArray *recent_scans;
  GtkWidget *recent_scan_menu;
} unique_app;

#ifdef HAVE_APPINDICATOR
static void systray_with_appindicator(GtkMenu *menu)
{
  AppIndicator *indicator;

  indicator = app_indicator_new("example-simple-client",
				"go-jump",
				APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_attention_icon(indicator, "indicator-messages-new");

  app_indicator_set_icon_full(indicator, SVG_ICON_PATH "/uhuru.svg", "uhuru");
  app_indicator_set_menu(indicator, menu);
}
#else
static void tray_icon_activated(GObject *tray_icon, gpointer popUpMenu)
{
  gtk_menu_popup(GTK_MENU(popUpMenu), NULL, NULL, gtk_status_icon_position_menu, tray_icon, 0, gtk_get_current_event_time());
}

static void tray_icon_popup(GtkStatusIcon *status_icon, guint button, guint32 activate_time, gpointer popUpMenu)
{
  gtk_menu_popup(GTK_MENU(popUpMenu), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
}

static void systray_with_status_icon(GtkMenu *menu)
{
  GtkStatusIcon *tray_icon = gtk_status_icon_new_from_file(PNG_ICON_PATH "/uhuru.png");

  gtk_status_icon_set_tooltip(tray_icon, "Uhuru antivirus");
  gtk_status_icon_set_visible(tray_icon, TRUE);

  g_signal_connect(GTK_STATUS_ICON(tray_icon), "activate", GTK_SIGNAL_FUNC(tray_icon_activated), menu);
  g_signal_connect(GTK_STATUS_ICON(tray_icon), "popup-menu", GTK_SIGNAL_FUNC(tray_icon_popup), menu);
}
#endif

void uhuru_app_init(void)
{
  struct systray_menu *systray_menu;

  unique_app.recent_scans = g_ptr_array_new();
  systray_menu = systray_menu_new();

#ifdef HAVE_APPINDICATOR
  systray_with_appindicator(GTK_MENU(systray_menu->main_menu));
#else
  systray_with_status_icon(systray_menu->main_menu);
#endif

  unique_app.recent_scan_menu = systray_menu->recent_scan_menu;

  notify_init("Uhuru");

  uhuru_dbus_start();
}

void uhuru_app_quit(void)
{
  uhuru_dbus_stop();

  notify_uninit();
}

static gboolean delete_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  struct scan_entry *e = (struct scan_entry *)data;

  fprintf(stderr, "deleting %p\n", e->dialog);

  return FALSE;
}

static void destroy_cb(GtkWidget *widget, gpointer data)
{
  struct scan_entry *e = (struct scan_entry *)data;

  fprintf(stderr, "destroying %p\n", e->dialog);

  e->dialog = NULL;
}

static void recent_scan_cb(gpointer data)
{
  struct scan_entry *entry = (struct scan_entry *)data;

  if (entry->dialog == NULL) {
    entry->dialog = scan_dialog_new(entry->model);

    /* g_signal_connect(e->dialog, "destroy", G_CALLBACK(gtk_widget_destroyed), &e->dialog); */
    g_signal_connect(entry->dialog->dialog_widget, "destroy", G_CALLBACK(destroy_cb), entry);
    g_signal_connect(entry->dialog->dialog_widget, "delete-event", G_CALLBACK (delete_cb), entry);
  }

  gtk_widget_show(GTK_WIDGET(entry->dialog->dialog_widget));
}

static struct scan_entry *scan_entry_new(const char *path)
{
  struct scan_entry *e;

  e = (struct scan_entry *)malloc(sizeof(struct scan_entry));

  e->model = g_object_new(SCAN_MODEL_TYPE, "scan-path", path, NULL);

  e->item = gtk_menu_item_new_with_label(path);
  gtk_widget_show(e->item);

  g_signal_connect_swapped(e->item, "activate", G_CALLBACK(recent_scan_cb), e);

  e->dialog = NULL;

  return e;
}

static void scan_show_notification(const char *path)
{
  GString *tmp;
  NotifyNotification *scan_notification;

  tmp = g_string_new("");
  g_string_printf(tmp, _("Scanning: %s"), path);
  scan_notification = notify_notification_new("Uhuru", tmp->str, "dialog-information");

  notify_notification_set_timeout(scan_notification, NOTIFY_EXPIRES_DEFAULT);

  notify_notification_show(scan_notification, NULL);
  
  g_object_unref(G_OBJECT(scan_notification));
  g_string_free(tmp, TRUE);
}

/*
  create a scan model (status init)
  insert it in recent scans
  if show_notification or show_all, show notification
  if show_dialog or show_all, create and show dialog
  start scan model (status in_progress)
*/
void uhuru_app_scan(const char *path, enum app_scan_show scan_show)
{
  struct scan_entry *entry;

  entry = scan_entry_new(path);
  g_ptr_array_insert(unique_app.recent_scans, 0, entry);

  gtk_menu_shell_insert(GTK_MENU_SHELL(unique_app.recent_scan_menu), entry->item, 0);

  if (scan_show == SHOW_NOTIFICATION || scan_show == SHOW_ALL)
    scan_show_notification(path);

  if (scan_show == SHOW_DIALOG || scan_show == SHOW_ALL) {
    entry->dialog = scan_dialog_new(entry->model);

    /* g_signal_connect(entry->dialog, "destroy", G_CALLBACK(gtk_widget_destroyed), &entry->dialog); */
    g_signal_connect(entry->dialog->dialog_widget, "delete-event", G_CALLBACK (delete_cb), entry);
    g_signal_connect(entry->dialog->dialog_widget, "destroy", G_CALLBACK(destroy_cb), entry);

    gtk_widget_show(GTK_WIDGET(entry->dialog->dialog_widget));
  }

  scan_model_scan(SCAN_MODEL(entry->model));
}
