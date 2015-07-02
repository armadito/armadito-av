#include "aboutdialog.h"

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

#include "ui/glade_aboutdialog.h"

GtkAboutDialog *about_dialog_new(void)
{
  GError *err = NULL;
  GtkBuilder *builder; 
  GtkAboutDialog *dialog;

  builder = gtk_builder_new ();
  if(gtk_builder_add_from_string(builder, aboutdialog_builder_def, strlen(aboutdialog_builder_def), &err) == 0) {
    fprintf(stderr, "Error adding build from string. Error: %s\n", err->message);

    exit(1);
  }

  dialog = GTK_ABOUT_DIALOG(gtk_builder_get_object(builder, "aboutDialog"));
  /* gtk_builder_connect_signals(builder, NULL); */

  g_object_unref(G_OBJECT(builder));
        
  return dialog;
}
