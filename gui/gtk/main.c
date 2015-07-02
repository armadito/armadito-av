#include "uhuru-linux-gui-config.h"
#include "app.h"
#include "utils/uhuru.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  bindtextdomain(GETTEXT_PACKAGE, PROGRAMNAME_LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  uhuru_open_thread_start();

  uhuru_app_init();

  gtk_main();

  uhuru_app_quit();

  uhuru_close(uhuru_handle());

  return 0;
}
