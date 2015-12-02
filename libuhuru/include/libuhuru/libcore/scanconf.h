#ifndef _LIBUHURU_LIBCORE_SCANCONF_H_
#define _LIBUHURU_LIBCORE_SCANCONF_H_

struct uhuru_scan_conf;

struct uhuru_scan_conf *uhuru_scan_conf_on_demand(void);

struct uhuru_scan_conf *uhuru_scan_conf_on_access(void);

void uhuru_scan_conf_white_list_directory(struct uhuru_scan_conf *c, const char *path);

int uhuru_scan_conf_is_white_listed(struct uhuru_scan_conf *c, const char *path);

void uhuru_scan_conf_add_mime_type(struct uhuru_scan_conf *c, const char *mime_type, const char *module_name, struct uhuru *u);

struct uhuru_module **uhuru_scan_conf_get_applicable_modules(struct uhuru_scan_conf *c, const char *mime_type);

void uhuru_scan_conf_max_file_size(struct uhuru_scan_conf *c, int max_file_size);

#endif
