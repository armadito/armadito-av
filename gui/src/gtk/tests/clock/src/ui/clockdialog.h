#ifndef _UI_CLOCKDIALOG_H_
#define _UI_CLOCKDIALOG_H_

#include <gtk/gtk.h>

struct clock_dialog *clock_dialog_new(void);

void clock_dialog_clock(struct clock_dialog *sd, guint period, int use_thread);

#endif
