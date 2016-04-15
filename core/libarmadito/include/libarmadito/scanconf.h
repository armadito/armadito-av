#ifndef _LIBARMADITO_SCANCONF_H_
#define _LIBARMADITO_SCANCONF_H_

struct a6o_scan_conf;

struct a6o_scan_conf *a6o_scan_conf_on_demand(void);

struct a6o_scan_conf *a6o_scan_conf_on_access(void);

void a6o_scan_conf_white_list_directory(struct a6o_scan_conf *c, const char *path);

int a6o_scan_conf_is_white_listed(struct a6o_scan_conf *c, const char *path);

void a6o_scan_conf_add_mime_type(struct a6o_scan_conf *c, const char *mime_type);

void a6o_scan_conf_add_module(struct a6o_scan_conf *c, const char *module_name, struct armadito *u);

struct a6o_module **a6o_scan_conf_get_applicable_modules(struct a6o_scan_conf *c, const char *mime_type);

void a6o_scan_conf_max_file_size(struct a6o_scan_conf *c, int max_file_size);

void a6o_scan_conf_free(struct a6o_scan_conf *scan_conf);

#endif
