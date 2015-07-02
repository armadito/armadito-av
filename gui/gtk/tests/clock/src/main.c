#include "ui/clockdialog.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void on_window_destroy(GtkObject *object, gpointer user_data)
{
  gtk_main_quit();
}

int main(int argc, char *argv[])
{
  struct clock_dialog *sd;
  guint period = 10 /* milliseconds */;
  int argp;
  int use_thread = FALSE;

  gtk_init (&argc, &argv);

  sd = clock_dialog_new();

  argp = 1;
  if (argc >= 2) {
    if (!strcmp(argv[argp], "--thread")) {
      use_thread = TRUE;
      argp++;
    }
    if (argp < argc)
      sscanf(argv[argp], "%d", &period);
  }

  fprintf(stderr, "starting clock: period = %d use_thread = %d\n", period, use_thread);

  clock_dialog_clock(sd, period, use_thread);

  printf("thread: %p\n", g_thread_self());

  gtk_main();

  return 0;
}
