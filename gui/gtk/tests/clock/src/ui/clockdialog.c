#include "clockdialog.h"
#include "ui/glade_clockdialog.h"
#include "model/clockmodel.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct clock_dialog {
  GtkWidget *dialog;

#if 0
  GtkEntry *hourCount, *minuteCount, *secondCount;
#endif
#if 1
  GtkLabel *hourCount, *minuteCount, *secondCount;
#endif
};

struct clock_dialog *clock_dialog_new(void)
{
  GError *err = NULL;
  GtkBuilder *builder; 
  GtkWidget *dialog;
  struct clock_dialog *sd;

  builder = gtk_builder_new ();
  if(gtk_builder_add_from_string(builder, clockdialog_builder_def, strlen(clockdialog_builder_def), &err) == 0) {
    fprintf(stderr, "Error adding build from string. Error: %s\n", err->message);
    exit(1);
  }

  dialog = GTK_WIDGET (gtk_builder_get_object (builder, "clockDialog"));
  gtk_builder_connect_signals(builder, NULL);

  sd = (struct clock_dialog *)malloc(sizeof(struct clock_dialog));

  sd->dialog = dialog;

#if 0
  sd->hourCount = GTK_ENTRY(gtk_builder_get_object(builder, "hourCount"));
  sd->minuteCount = GTK_ENTRY(gtk_builder_get_object(builder, "minuteCount"));
  sd->secondCount = GTK_ENTRY(gtk_builder_get_object(builder, "secondCount"));
#endif
#if 1
  sd->hourCount = GTK_LABEL(gtk_builder_get_object(builder, "hourCount"));
  sd->minuteCount = GTK_LABEL(gtk_builder_get_object(builder, "minuteCount"));
  sd->secondCount = GTK_LABEL(gtk_builder_get_object(builder, "secondCount"));
#endif
  
  gtk_widget_show(sd->dialog);

  g_object_unref(G_OBJECT(builder));
        
  return sd;
}

static void count_changed_cb(GObject *obj, guint counter_type, guint counter_value, gpointer user_data)
{
  struct clock_dialog *sd = (struct clock_dialog *)user_data;
#if 0
  GtkEntry *entry;
#endif
#if 1
  GtkLabel *entry;
#endif
  char *tmp;

  /* fprintf(stderr, "count changed %s %d\n", clock_counter_type_str(counter_type), counter_value); */

  switch(counter_type) {
  case HOUR_COUNTER:
    entry = sd->hourCount;
    break;
  case MINUTE_COUNTER:
    entry = sd->minuteCount;
    break;
  case SECOND_COUNTER:
    entry = sd->secondCount;
    break;
  }

  tmp = (char *)malloc(64);
  sprintf(tmp, "%d", counter_value);

#if 1
  gdk_threads_enter();
#endif
#if 0
  gtk_entry_set_text(entry, tmp);
#endif
#if 1
  gtk_label_set_text(entry, tmp);
#endif
#if 1
  gdk_threads_leave();
#endif

  free(tmp);
}

void clock_dialog_clock(struct clock_dialog *sd, guint period, int use_thread)
{
  GObject *model;

  model = g_object_new(CLOCK_MODEL_TYPE, "period", period, NULL);

  g_signal_connect(model, "count_changed", (GCallback)count_changed_cb, sd);

  clock_model_clock(CLOCK_MODEL(model), use_thread);
}
