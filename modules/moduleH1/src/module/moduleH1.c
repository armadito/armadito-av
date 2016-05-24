#include <libarmadito.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "osdeps.h"
#include "h1-errors.h"
#include "h1-tatic.h"

/* under re-implementation */

struct moduleH1_data {
	const char *tmp_dir;
	int late_days;
	int critical_days;
};

static enum a6o_mod_status moduleH1_init(struct a6o_module *module)
{
	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status moduleH1_post_init(struct a6o_module *module)
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
	len = strlen(bases_location) + 1 + strlen("moduleH1\\windows\\Database_malsain_2.zip") + 1;
	modelMalwareEat = calloc(len + 1, sizeof(char));
	modelMalwareEat[len] = '\0';
	sprintf_s(modelMalwareEat, len, "%s%cmoduleH1\\windows\\Database_malsain_2.zip", bases_location, a6o_path_sep());

	// modelMalwareIat
	len = strlen(bases_location) + 1 + strlen("moduleH1\\windows\\Database_malsain_1.zip") + 1;
	modelMalwareIat = calloc(len + 1, sizeof(char));
	modelMalwareIat[len] = '\0';
	sprintf_s(modelMalwareIat, len, "%s%cmoduleH1\\windows\\Database_malsain_1.zip", bases_location, a6o_path_sep());	

	// modelNotMalwareEat
	len = strlen(bases_location) + 1 + strlen("moduleH1\\windows\\Database_sain_2.zip") + 1;
	modelNotMalwareEat = calloc(len + 1, sizeof(char));
	modelNotMalwareEat[len] = '\0';
	sprintf_s(modelNotMalwareEat, len, "%s%cmoduleH1\\windows\\Database_sain_2.zip", bases_location, a6o_path_sep());	

	// modelNotMalwareIat
	len = strlen(bases_location) + 1 + strlen("moduleH1\\windows\\Database_sain_1.zip") + 1;
	modelNotMalwareIat = calloc(len + 1, sizeof(char));
	modelNotMalwareIat[len] = '\0';
	sprintf_s(modelNotMalwareIat, len, "%s%cmoduleH1\\windows\\Database_sain_1.zip", bases_location, a6o_path_sep());

	// databaseEat
	len = strlen(bases_location) + 1 + strlen("moduleH1\\windows\\database_2.dat") + 1;
	databaseEat = calloc(len + 1, sizeof(char));
	databaseEat[len] = '\0';
	sprintf_s(databaseEat, len, "%s%cmoduleH1\\windows\\database_2.dat", bases_location, a6o_path_sep());

	// databaseIat
	len = strlen(bases_location) + 1 + strlen("moduleH1\\windows\\database_1.dat") + 1;
	databaseIat = calloc(len + 1, sizeof(char));
	databaseIat[len] = '\0';
	sprintf_s(databaseIat, len, "%s%cmoduleH1\\windows\\database_1.dat", bases_location, a6o_path_sep());

	// databaseTFIDFInf
	len = strlen(bases_location) + 1 + strlen("moduleH1\\windows\\DBI_inf.dat") + 1;
	databaseTFIDFInf = calloc(len + 1, sizeof(char));
	databaseTFIDFInf[len] = '\0';
	sprintf_s(databaseTFIDFInf, len, "%s%cmoduleH1\\windows\\DBI_inf.dat", bases_location, a6o_path_sep());

	// databaseTFIDFSain
	len = strlen(bases_location) + 1 + strlen("moduleH1\\windows\\DBI_sain.dat") + 1;
	databaseTFIDFSain = calloc(len + 1, sizeof(char));
	databaseTFIDFSain[len] = '\0';
	sprintf_s(databaseTFIDFSain, len, "%s%cmoduleH1\\windows\\DBI_sain.dat", bases_location, a6o_path_sep());

	//printf("[+] Debug :: module H1 database file = [%s]\n", databaseEat);

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

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "module H1 PE databases loaded from %s\\moduleH1\\windows \n", bases_location);

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

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "loading module H1 ELF databases from " MODULEH1_DBDIR "/linux");

	if (initDB(MODULEH1_DBDIR "/linux/database.elfdata",
			MODULEH1_DBDIR "/linux/db_malicious.zip",
			MODULEH1_DBDIR "/linux/db_safe.zip",
			MODULEH1_DBDIR "/linux/tfidf_m.dat",
			MODULEH1_DBDIR "/linux/tfidf_s.dat") != 0)
		return ARMADITO_MOD_INIT_ERROR;

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "module H1 ELF databases loaded from " MODULEH1_DBDIR "/linux");

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "loading module H1 PE databases from " MODULEH1_DBDIR "/windows");
	if (initDatabases(MODULEH1_DBDIR "/windows/Database_malsain_2.zip",
			MODULEH1_DBDIR "/windows/Database_malsain_1.zip",
			MODULEH1_DBDIR "/windows/Database_sain_2.zip",
			MODULEH1_DBDIR "/windows/Database_sain_1.zip",
			MODULEH1_DBDIR "/windows/database_2.dat",
			MODULEH1_DBDIR "/windows/database_1.dat",
			MODULEH1_DBDIR "/windows/DBI_inf.dat",
			MODULEH1_DBDIR "/windows/DBI_sain.dat") != 0)
		return ARMADITO_MOD_INIT_ERROR;

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, "module H1 PE databases loaded from %s " MODULEH1_DBDIR "/windows");
	
#endif



	return ARMADITO_MOD_OK;
}

// We receive a file descriptor
static enum a6o_file_status moduleH1_scan(struct a6o_module *module, int fd, const char *path, const char *mime_type, char **pmod_report)
{
	ERROR_CODE e = UH_NULL;
	const char *virus_name = NULL;

	//printf("[i] Debug :: module H1 scan :: mime-type = %s\n",mime_type);

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

	printf("moduleH1 internal error : %s \n", error_code_str(e));
	return ARMADITO_IERROR;
}

static enum a6o_mod_status moduleH1_close(struct a6o_module *module)
{
	return ARMADITO_MOD_OK;
}

/* FIXME: add bases status */
static enum a6o_update_status moduleH1_info(struct a6o_module *module, struct a6o_module_info *info)
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

static const char *moduleH1_mime_types[] = {
	"application/x-executable",
	"application/x-object",
	"application/x-sharedlib",
	"application/x-dosexec",
	"application/x-msdos-program",
	"application/x-msdownload",
	NULL,
};

struct a6o_module module = {
	.init_fun = moduleH1_init,
	.conf_table = NULL,
	.post_init_fun = moduleH1_post_init,
	.scan_fun = moduleH1_scan,
	.close_fun = moduleH1_close,
	.info_fun = moduleH1_info,
	.supported_mime_types = moduleH1_mime_types,
	.name = "moduleH1",
	.size = sizeof(struct moduleH1_data),
};
