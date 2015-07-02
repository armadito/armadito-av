#ifndef _UI_SCANCHOOSER_H_
#define _UI_SCANCHOOSER_H_

#include <gtk/gtk.h>

struct scan_chooser {
  GtkFileChooserDialog *file_chooser;
};

struct scan_chooser *scan_chooser_new(void);

char *scan_chooser_choose(struct scan_chooser *sc);

#endif
