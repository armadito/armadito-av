#include <libarmadito.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "osdeps.h"
#include "uh_errors.h"
#include "UhuruStatic.h"

/* under re-implementation */

struct module5_2_data {
	const char *tmp_dir;
	int late_days;
	int critical_days;
};

static enum a6o_mod_status module5_2_init(struct a6o_module *module)
{
	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status module5_2_post_init(struct a6o_module *module)
{
#ifndef WIN32
	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "loading module 5.2 ELF databases from " MODULE5_2_DBDIR "/linux");

	if (initDB(MODULE5_2_DBDIR "/linux/database.elfdata",
			MODULE5_2_DBDIR "/linux/db_malicious.zip",
			MODULE5_2_DBDIR "/linux/db_safe.zip",
			MODULE5_2_DBDIR "/linux/tfidf_m.dat",
			MODULE5_2_DBDIR "/linux/tfidf_s.dat") != 0)
		return ARMADITO_MOD_INIT_ERROR;

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "module 5.2 ELF databases loaded from " MODULE5_2_DBDIR "/linux");
#endif

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "loading module 5.2 PE databases from " MODULE5_2_DBDIR "/windows");
	if (initDatabases(MODULE5_2_DBDIR "/windows/Database_malsain_2.zip",
				MODULE5_2_DBDIR "/windows/Database_malsain_1.zip",
				MODULE5_2_DBDIR "/windows/Database_sain_2.zip",
				MODULE5_2_DBDIR "/windows/Database_sain_1.zip",
				MODULE5_2_DBDIR "/windows/database_2.dat",
				MODULE5_2_DBDIR "/windows/database_1.dat",
				MODULE5_2_DBDIR "/windows/DBI_inf.dat",
				MODULE5_2_DBDIR "/windows/DBI_sain.dat") != 0)
		return ARMADITO_MOD_INIT_ERROR;

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "module 5.2 PE databases loaded from " MODULE5_2_DBDIR "/windows");

	return ARMADITO_MOD_OK;
}

// We receive a file descriptor
static enum a6o_file_status module5_2_scan(struct a6o_module *module, int fd, const char *path, const char *mime_type, char **pmod_report)
{
	ERROR_CODE e = UH_NULL;
	const char *virus_name = NULL;

	//printf("[i] Debug :: module 5_2_scan :: mime-type = %s\n",mime_type);

	// TODO change strcmp by strncmp
	// (FD) why????
	if (!strcmp(mime_type, "application/x-sharedlib")
		|| !strcmp(mime_type, "application/x-object")
		|| !strcmp(mime_type, "application/x-executable")) {
		e = analyseElfFile(fd, (char *)path);
		if (e == UH_MALWARE)
			virus_name = "Linux.Heuristic.Malware.Generic";
	} else if (!strcmp(mime_type, "application/x-dosexec")
		|| !strcmp(mime_type, "application/x-msdownload")) {
		e = fileAnalysis(fd, (char *)path);
		if (e == UH_MALWARE)
			virus_name = "Win.Heuristic.Malware.Generic";
	}

	switch(e) {
	case UH_MALWARE:
		/* even if virus_name is a statically allocated string, it must be returned in a dynamically allocated string */
		/* because it will be free()d by the calling code */
		*pmod_report = os_strdup(virus_name);
		return ARMADITO_MALWARE;
	case UH_NOT_MALWARE:
		return ARMADITO_CLEAN;
	case UH_NOT_DECIDED:
	case UH_DOUBTFUL:
		return ARMADITO_UNDECIDED;
	}

	return ARMADITO_IERROR;
}

static enum a6o_mod_status module5_2_close(struct a6o_module *module)
{
	return ARMADITO_MOD_OK;
}

/* FIXME: one day, add bases status */
static enum a6o_update_status module5_2_info(struct a6o_module *module, struct a6o_module_info *info)
{
	time_t ts = 0;
	struct tm timeptr = {0, 30, 8, 1, 8, 114}; // 01/09/2014 9:30
	
	info->base_infos = (struct a6o_base_info **)malloc(sizeof(struct a6o_base_info *));
	info->base_infos[0] = NULL;
	
	info->update_date = os_strdup("2014-09-01T09:30:00Z");

	ts = mktime(&timeptr);
	info->timestamp = ts;

	return ARMADITO_UPDATE_OK;
}

static const char *module5_2_mime_types[] = {
	"application/x-executable",
	"application/x-object",
	"application/x-sharedlib",
	"application/x-dosexec",
	"application/x-msdos-program",
	"application/x-msdownload",
	NULL,
};

struct a6o_module module = {
	.init_fun = module5_2_init,
	.conf_table = NULL,
	.post_init_fun = module5_2_post_init,
	.scan_fun = module5_2_scan,
	.close_fun = module5_2_close,
	.info_fun = module5_2_info,
	.supported_mime_types = module5_2_mime_types,
	.name = "module5_2",
	.size = sizeof(struct module5_2_data),
};
