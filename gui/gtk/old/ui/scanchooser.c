#include "scanchooser.h"
#include "ui/glade_scanchooser.h"

#include <stdlib.h>
#include <string.h>

struct scan_chooser *scan_chooser_new(void)
{
  GError *err = NULL;
  GtkBuilder *builder; 
  GtkFileChooserDialog *file_chooser;
  struct scan_chooser *sc;

  builder = gtk_builder_new ();
  if(gtk_builder_add_from_string(builder, scanchooser_builder_def, strlen(scanchooser_builder_def), &err) == 0) {
    fprintf(stderr, "Error adding build from string. Error: %s\n", err->message);

    exit(1);
  }

  file_chooser = GTK_FILE_CHOOSER_DIALOG(gtk_builder_get_object(builder, "scanChooser"));
  gtk_builder_connect_signals(builder, NULL);

  sc = (struct scan_chooser *)malloc(sizeof(struct scan_chooser));

  sc->file_chooser = file_chooser;

  gtk_widget_show(GTK_WIDGET(sc->file_chooser));

  g_object_unref(G_OBJECT(builder));
        
  return sc;
}

char *scan_chooser_choose(struct scan_chooser *sc)
{
  if (gtk_dialog_run(GTK_DIALOG(sc->file_chooser)) == 0) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(sc->file_chooser));

    gtk_widget_destroy(GTK_WIDGET(sc->file_chooser));

    return filename;
  }
 
  return NULL;
}
