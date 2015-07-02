#include <gtk/gtk.h>
#include <stdlib.h>

void 
on_window_destroy (GtkObject *object, gpointer user_data)
{
    gtk_main_quit ();
}

int
main (int argc, char *argv[])
{
    GtkBuilder      *builder; 
    GtkWidget       *window;
    GtkEntry *toScanCount, *scannedCount, *malwareCount, *suspiciousCount, *cleanCount;
    GError *err = NULL;
    char *tmp;
    int count = 42;

    gtk_init (&argc, &argv);

    builder = gtk_builder_new ();
    if(gtk_builder_add_from_file(builder, "/home/francois/projects/uhuru/git/uhuru-linux-gui/src/gtk/scanwindow.glade", &err) == 0) {
      /* Print out the error. You can use GLib's message logging */
      fprintf(stderr, "Error adding build from file. Error: %s\n", err->message);
      /* Your error handling code goes here */
      exit(1);
    }
    /* gtk_builder_add_from_file (builder, "tutorial.glade", NULL); */
    window = GTK_WIDGET (gtk_builder_get_object (builder, "dialog1"));
    gtk_builder_connect_signals (builder, NULL);

    toScanCount = GTK_ENTRY(gtk_builder_get_object(builder, "toScanCount"));
    scannedCount = GTK_ENTRY(gtk_builder_get_object(builder, "scannedCount"));
    malwareCount = GTK_ENTRY(gtk_builder_get_object(builder, "malwareCount"));
    suspiciousCount = GTK_ENTRY(gtk_builder_get_object(builder, "suspiciousCount"));
    cleanCount = GTK_ENTRY(gtk_builder_get_object(builder, "cleanCount"));

    tmp = (char *)malloc(64);
    sprintf(tmp, "%d", count++);
    gtk_entry_set_text(toScanCount, tmp);

    sprintf(tmp, "%d", count++);
    gtk_entry_set_text(scannedCount, tmp);

    sprintf(tmp, "%d", count++);
    gtk_entry_set_text(malwareCount, tmp);

    sprintf(tmp, "%d", count++);
    gtk_entry_set_text(suspiciousCount, tmp);

    sprintf(tmp, "%d", count++);
    gtk_entry_set_text(cleanCount, tmp);

    g_object_unref (G_OBJECT (builder));
        
    gtk_widget_show (window);                
    gtk_main ();

    return 0;
}
