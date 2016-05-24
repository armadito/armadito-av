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


#ifdef _WIN32

	const char * bases_location = NULL;
	int len;

	char* modelMalwareEat = NULL;
	char* modelMalwareIat = NULL;
	char* modelNotMalwareEat = NULL;
	char* modelNotMalwareIat = NULL;
	char* databaseEat = NULL;
	char* databaseIat = NULL;
	char* databaseTFIDFInf = NULL;
	char* databaseTFIDFSain = NULL;

	/*build db directory complete path*/
	bases_location = a6o_std_path(BASES_LOCATION);

	// modelMalwareEat
	len = strlen(bases_location) + 1 + strlen("module5_2\\windows\\Database_malsain_2.zip") + 1;
	modelMalwareEat = calloc(len + 1, sizeof(char));
	modelMalwareEat[len] = '\0';
	sprintf_s(modelMalwareEat, len, "%s%cmodule5_2\\windows\\Database_malsain_2.zip", bases_location, a6o_path_sep());

	// modelMalwareIat
	len = strlen(bases_location) + 1 + strlen("module5_2\\windows\\Database_malsain_1.zip") + 1;
	modelMalwareIat = calloc(len + 1, sizeof(char));
	modelMalwareIat[len] = '\0';
	sprintf_s(modelMalwareIat, len, "%s%cmodule5_2\\windows\\Database_malsain_1.zip", bases_location, a6o_path_sep());	

	// modelNotMalwareEat
	len = strlen(bases_location) + 1 + strlen("module5_2\\windows\\Database_sain_2.zip") + 1;
	modelNotMalwareEat = calloc(len + 1, sizeof(char));
	modelNotMalwareEat[len] = '\0';
	sprintf_s(modelNotMalwareEat, len, "%s%cmodule5_2\\windows\\Database_sain_2.zip", bases_location, a6o_path_sep());	

	// modelNotMalwareIat
	len = strlen(bases_location) + 1 + strlen("module5_2\\windows\\Database_sain_1.zip") + 1;
	modelNotMalwareIat = calloc(len + 1, sizeof(char));
	modelNotMalwareIat[len] = '\0';
	sprintf_s(modelNotMalwareIat, len, "%s%cmodule5_2\\windows\\Database_sain_1.zip", bases_location, a6o_path_sep());

	// databaseEat
	len = strlen(bases_location) + 1 + strlen("module5_2\\windows\\database_2.dat") + 1;
	databaseEat = calloc(len + 1, sizeof(char));
	databaseEat[len] = '\0';
	sprintf_s(databaseEat, len, "%s%cmodule5_2\\windows\\database_2.dat", bases_location, a6o_path_sep());

	// databaseIat
	len = strlen(bases_location) + 1 + strlen("module5_2\\windows\\database_1.dat") + 1;
	databaseIat = calloc(len + 1, sizeof(char));
	databaseIat[len] = '\0';
	sprintf_s(databaseIat, len, "%s%cmodule5_2\\windows\\database_1.dat", bases_location, a6o_path_sep());

	// databaseTFIDFInf
	len = strlen(bases_location) + 1 + strlen("module5_2\\windows\\DBI_inf.dat") + 1;
	databaseTFIDFInf = calloc(len + 1, sizeof(char));
	databaseTFIDFInf[len] = '\0';
	sprintf_s(databaseTFIDFInf, len, "%s%cmodule5_2\\windows\\DBI_inf.dat", bases_location, a6o_path_sep());

	// databaseTFIDFSain
	len = strlen(bases_location) + 1 + strlen("module5_2\\windows\\DBI_sain.dat") + 1;
	databaseTFIDFSain = calloc(len + 1, sizeof(char));
	databaseTFIDFSain[len] = '\0';
	sprintf_s(databaseTFIDFSain, len, "%s%cmodule5_2\\windows\\DBI_sain.dat", bases_location, a6o_path_sep());

	//printf("[+] Debug :: module 5.2 database file = [%s]\n", databaseEat);

	/* initDatabase function extension :: add db location as first parameter */
	if (initDatabases(modelMalwareEat,
			modelMalwareIat,
			modelNotMalwareEat,
			modelNotMalwareIat,
			databaseEat,
			databaseIat,
			databaseTFIDFInf,
			databaseTFIDFSain) != 0)
		return ARMADITO_MOD_INIT_ERROR;

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "module 5.2 PE databases loaded from %s\\module5_2\\windows \n", bases_location);

	free(modelMalwareEat);
	free(modelMalwareIat);
	free(modelNotMalwareEat);
	free(modelNotMalwareIat);
	free(databaseEat);
	free(databaseIat);
	free(databaseTFIDFInf);
	free(databaseTFIDFSain);
	free(bases_location);
	

#else

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "loading module 5.2 ELF databases from " MODULE5_2_DBDIR "/linux");

	if (initDB(MODULE5_2_DBDIR "/linux/database.elfdata",
			MODULE5_2_DBDIR "/linux/db_malicious.zip",
			MODULE5_2_DBDIR "/linux/db_safe.zip",
			MODULE5_2_DBDIR "/linux/tfidf_m.dat",
			MODULE5_2_DBDIR "/linux/tfidf_s.dat") != 0)
		return ARMADITO_MOD_INIT_ERROR;

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "module 5.2 ELF databases loaded from " MODULE5_2_DBDIR "/linux");

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

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "module 5.2 PE databases loaded from %s " MODULE5_2_DBDIR "/windows");
	
#endif



	return ARMADITO_MOD_OK;
}

// We receive a file descriptor
static enum a6o_file_status module5_2_scan(struct a6o_module *module, int fd, const char *path, const char *mime_type, char **pmod_report)
{
	ERROR_CODE e = UH_NULL;
	const char *virus_name = NULL;

	//printf("[i] Debug :: module 5_2_scan :: mime-type = %s\n",mime_type);

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

	printf("module5_2 internal error : %s \n", error_code_str(e));
	return ARMADITO_IERROR;
}

static enum a6o_mod_status module5_2_close(struct a6o_module *module)
{
	return ARMADITO_MOD_OK;
}

/* FIXME: add bases status */
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
