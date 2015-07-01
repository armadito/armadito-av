#include "systraymenu.h"
#include "app.h"
#include "aboutdialog.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void scan_cb(gpointer data)
{
  struct scan_dialog *sd;
  char *path;
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new(_("Scan"),
				       NULL,
				       GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				       NULL);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT) {
    gtk_widget_destroy(dialog);
    return;
  }

  path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
  if (path == NULL) {
    gtk_widget_destroy(dialog);
    return;
  }

  gtk_widget_destroy(dialog);

  uhuru_app_scan(path, SHOW_DIALOG);

  g_free(path);
}

static void about_cb(gpointer data)
{
  GtkWidget *dialog = GTK_WIDGET(about_dialog_new());

  g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);

  gtk_widget_show(dialog);
}

struct systray_menu *systray_menu_new(void)
{
  GtkWidget *scan_item;
  GtkWidget *recent_scan_item;
  GtkWidget *recent_scan_menu;
  GtkWidget *about_item;
  struct systray_menu *sm;

  sm = (struct systray_menu *)malloc(sizeof(struct systray_menu));

  sm->main_menu = gtk_menu_new();

  scan_item = gtk_menu_item_new_with_label(_("Scan"));
  gtk_menu_shell_append(GTK_MENU_SHELL(sm->main_menu), scan_item);
  gtk_widget_show(scan_item);

  recent_scan_item = gtk_menu_item_new_with_label(_("Recent scans"));
  gtk_menu_shell_append(GTK_MENU_SHELL(sm->main_menu), recent_scan_item);
  gtk_widget_show(recent_scan_item);

  sm->recent_scan_menu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(recent_scan_item), sm->recent_scan_menu);

#if 0
  {
    GtkWidget *foo_item;

    foo_item = gtk_menu_item_new_with_label(_("Foo"));
    gtk_menu_shell_append(GTK_MENU_SHELL(recent_scan_menu), foo_item);
    gtk_widget_show(foo_item);

    foo_item = gtk_menu_item_new_with_label(_("Bar"));
    gtk_menu_shell_append(GTK_MENU_SHELL(recent_scan_menu), foo_item);
    gtk_widget_show(foo_item);
  }
#endif
        
  about_item = gtk_menu_item_new_with_label(_("About"));
  gtk_menu_shell_append(GTK_MENU_SHELL(sm->main_menu), about_item);
  gtk_widget_show(about_item);

  g_signal_connect_swapped(scan_item, "activate", G_CALLBACK(scan_cb), (gpointer)"scan");
  g_signal_connect_swapped(about_item, "activate", G_CALLBACK(about_cb), (gpointer)"about");

  return sm;
}

