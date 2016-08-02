/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "libarmadito-config.h"

#include <assert.h>
#include <getopt.h>
#include <errno.h>
#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "apiclient.h"

#define DEFAULT_PORT          8888
#define S_DEFAULT_PORT        "8888"

#define PROGRAM_NAME "armadito-scan"
#define PROGRAM_VERSION PACKAGE_VERSION

struct info_options {
	unsigned short port;
};

static struct option info_option_defs[] = {
	{"help",      no_argument,        0, 'h'},
	{"version",   no_argument,        0, 'V'},
	{"port",      required_argument,  0, 'p'},
	{0, 0, 0, 0}
};

struct base_info {
	const char *name;
	time_t base_update_ts;
	const char *version;
	unsigned int signature_count;
	const char *full_path;
};

struct module_info {
	const char *name;
	const char *mod_status;
	time_t mod_update_ts;
	/* NULL terminated array of pointers to struct base_info */
	struct base_info **base_infos;
};

struct status_event {
	const char *global_status;
	/* NULL terminated array of pointers to struct a6o_module_info */
	struct module_info **module_infos;
	int n_modules;
	int alloc_modules;
};

static void version(void)
{
	printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
	exit(EXIT_SUCCESS);
}

static void usage(void)
{
	fprintf(stderr, "usage: armadito-info [options]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Armadito antivirus information\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  --help  -h                    print help and quit\n");
	fprintf(stderr, "  --version -V                  print program version\n");
	fprintf(stderr, "  --port=PORT | -p PORT         TCP port to connect to Armadito-AV (default is " S_DEFAULT_PORT ")\n");
	fprintf(stderr, "\n");

	exit(1);
}

static void parse_options(int argc, char **argv, struct info_options *opts)
{
	opts->port = DEFAULT_PORT;

	while (1) {
		int c, option_index = 0;

		c = getopt_long(argc, argv, "hVp:", info_option_defs, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 'h': /* help */
			usage();
			break;
		case 'V': /* version */
			version();
			break;
		case 'p': /* port */
			opts->port = (unsigned short)atoi(optarg);
			break;
		case '?':
			/* getopt_long already printed an error message. */
			break;
		default:
			abort ();
		}
	}

	/* the command does not take arguments, just options */
	if (optind < argc)
		usage();
}

#if 0
static struct info *info_new(void)
{
	struct info *info = malloc(sizeof(struct info));

	info->global_status = NULL;

	info->n_modules = 0;
	info->alloc_modules = 1;
	info->module_infos = malloc(sizeof(struct module_info) * info->alloc_modules);
	info->module_infos[info->n_modules] = NULL;

	return info;
}

static void info_append_module(struct info *info, struct module_info *mod_inf)
{
	if (info->n_modules >= info->alloc_modules - 1) {
		info->alloc_modules *= 2;
		info->module_infos = realloc(info->module_infos, sizeof(struct module_info) * info->alloc_modules);
	}

	info->module_infos[info->n_modules] = mod_inf;
	info->n_modules++;
	info->module_infos[info->n_modules] = NULL;
}

static void info_free(struct info *info)
{
	/* FIXME: free all the fields */
	free(info);
}

static void info_save_to_stdout(struct info *info)
{
	struct module_info **m;
	struct base_info **b;

	printf( "--- Armadito info --- \n");
	printf( "Update global status : %s\n", info->global_status);
	if (info->module_infos != NULL) {
		for(m = info->module_infos; *m != NULL; m++){
			printf( "Module %s \n", (*m)->name );
			printf( "- Update status : %s\n", (*m)->mod_status);
			printf( "- Update date : %s \n", (*m)->update_date );

			if ((*m)->base_infos != NULL) {
				for(b = (*m)->base_infos; *b != NULL; b++){
					printf( "-- Base %s \n", (*b)->name );
					printf( "--- Update date : %s \n", (*b)->date );
					printf( "--- Version : %s \n", (*b)->version );
					printf( "--- Signature count : %d \n", (*b)->signature_count );
					printf( "--- Full path : %s \n", (*b)->full_path );
				}
			}
		}
	}
}
#endif

static void do_info(struct info_options *opts, struct api_client *client)
{
	struct json_object *j_response;
	int end_of_info = 0;
	struct status_event ev;

	api_client_register(client);

	api_client_call(client, "/status", NULL, NULL);

	while (!end_of_info) {
		j_response = NULL;
		api_client_call(client, "/event", NULL, &j_response);
		if (j_response == NULL)
			continue;

		json_object_to_file("/dev/stderr", j_response);
		fprintf(stderr, "\n");

#if 0
		switch(process_json_event(j_response, &ev)) {
		case DETECTION_EVENT:
			printf("%s: %s [%s - %s] (action %s)\n",
				ev.content.d.path,
				ev.content.d.scan_status,
				ev.content.d.module_name,
				ev.content.d.module_report,
				ev.content.d.scan_action);
			break;
		case ON_DEMAND_COMPLETED_EVENT:
			end_of_info = 1;
			if (!opts->no_summary) {
				printf("\nSCAN SUMMARY:\n");
				printf("scanned files     : %d\n", ev.content.c.total_scanned_count);
				printf("malware files     : %d\n", ev.content.c.total_malware_count);
				printf("suspicious files  : %d\n", ev.content.c.total_suspicious_count);
			}
			break;
		}
#endif
		end_of_info = 1;
	}

	api_client_unregister(client);
}

int main(int argc, char **argv)
{
	struct info_options *opts = (struct info_options *)malloc(sizeof(struct info_options));
	struct api_client *client;

	parse_options(argc, argv, opts);

	client = api_client_new(opts->port);

	do_info(opts, client);

	return 0;
}
