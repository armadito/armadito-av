#ifndef _UI_APP_H_
#define _UI_APP_H_

void uhuru_app_init(void);

void uhuru_app_quit(void);

enum app_scan_show {
  SHOW_NONE = 0,
  SHOW_NOTIFICATION = 1,
  SHOW_DIALOG = 2,
  SHOW_ALL = 3,
};

void uhuru_app_scan(const char *path, enum app_scan_show scan_show);

#endif
