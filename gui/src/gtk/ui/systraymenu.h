#ifndef _UI_SYSTRAYMENU_H_
#define _UI_SYSTRAYMENU_H_

#include <gtk/gtk.h>

struct systray_menu {
  GtkWidget *main_menu;
  GtkWidget *recent_scan_menu;
};

struct systray_menu *systray_menu_new(void);

#endif
